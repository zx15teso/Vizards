// #if 0
/*
 * Aufgabe 2 - Gitter Anzeigen
Nachdem Sie nun ein Gitter erzeugt haben, soll es auch angezeigt werden. Hierzu sollen
Sie einen VisAlgorithm schreiben, der je nach Auswahl die Gitterlinien des Grids oder
die Seitenfl ̈achen der Gitterzellen anzeigt (→ beides implementieren). Es soll entweder das
gesamte Grid angezeigt werden, oder nur eine einzelne Zelle (→ beides implementieren), je
nachdem was der Nutzer ausw ̈ahlt. Als Input f ̈ur den Algorithmus dient das anzuzeigende
Grid, die Farbe f ̈ur die Gitterlinien/Seitenfl ̈achen, der Index der anzuzeigenden Zelle,
sowie eine Option f ̈ur die Fallunterscheidungen.
Das Tutorial Fields, Grids, and Cells zeigt ihnen, wie sie in FAnToM auf ein Grid
und dessen Zellen zugreifen k ̈onnen. Ebenfalls hilfreich sind die Dokumentationen zu den
Klassen Grid ,und Cell (und deren Primitive: POINT, LINE, TRIANGLE, etc.). Wie in FAn-
ToM Grafik erzeugt und angezeigt wird, zeigt Ihnen das Tutorial Algorithm Graphics.
Vergleichen Sie Ihre Ergebnisse mit Hilfe des Show Grid-Plugins. Bitte beachten Sie, dass
verschiedene Zellen unterschiedlich gezeichnet werden m ̈ussen. F ̈ur die Umsetzung dieser
Aufgabe ist die Beachtung von Zelltypen, welche in der 1. Aufgabe verwendet wurden,
ausreichend. Zum  ̈uberpr ̈ufen der Ergebnisse verwenden Sie die Gitter aus der 1. Aufgabe
oder laden Sie einen der Test-Datens ̈atze mit dem load VTK Plugin ein.
HINWEIS:  ̈Uberpr ̈ufen Sie die Eingaben des Benutzers, sodass Fehler aufgrund von
unzul ̈assigen Werten vermieden werden. Bedenken Sie, dass wie im Beispiel der Stadt aus
Abbildung 2 eine Linie selbst eine Zelle darstellt und bei der Flaechendarstellung mit
gezeigt werden sollte.                 add<std::string>("Display Mode", "Geben Sie 'Lines' oder 'Faces' ein", "Lines");

muss aus der api das ziehen:

            // The Drawable object defines the input streams for the shaders.
            // Vertex- and IndexBuffers as well as Uniforms can be defined as seen below.
            std::shared_ptr< graphics::Drawable > simpleDrawable = system.makePrimitive(
                graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                    .vertexBuffer( "position", system.makeBuffer( tri ) )
                    .vertexBuffer( "normal", system.makeBuffer( norm ) )
                    .indexBuffer( system.makeIndexBuffer( indices ) )
                    .uniform( "color", Color( 0.5, 0.7, 0.1 ) )
                    .renderOption( graphics::RenderOption::Blend, true )
                    .boundingSphere( bs ),
                system.makeProgramFromFiles( resourcePath + "shader/surface/phong/singleColor/vertex.glsl",
                                             resourcePath + "shader/surface/phong/singleColor/fragment.glsl" ) );

            setGraphics( "simpleDrawable", simpleDrawable );

beachte: muss aus Gittern aus 1.1 und 1.2 die lines nehmen -> beachte dabei, dass bei vierecken 4 punkte gegeben werden, welche durch die indizes bestimmt sind, wo genau die
2 indizes bilden und auf die vertexes abbilden?!
Vertices sind variable stellen an der gpu -> indexbuffer für bei vierecken 4 punkte gegeben werden, welche durch die indizes bestimmt sind, wo genau die dreiecke (die nur verarbeitet werden können)
shader sind programme auf gpu
shader in opengl?!?!?!?

Wissen:
- Vertex ist Punkt im 3D-Raum mit Koordinaten (x,y,z) und kann Attribute erhalten
- Shader sind Programme, die auf  GPU ausgeführt werden - dienen zur Darstellung von 3D-Grafik und berechnen visuelle Effekte wie Beleuchtung, Schatten, Texturen und Oberflächenreflexionen
    -> Vertex Shader: verarbeitet die Eckpunkte (Vertices) eines 3D-Modells durch Transformation der 3D-Koordinaten in 2D-Koordinaten (Projektion auf den Bildschirm) und Berechnung der Attribute für jeden Vertex
- Vertex Buffer: Datenpuffer, der auf der Grafikkarte (GPU) gespeichert wird und Informationen über die Eckpunkte (Vertices) eines 3D-Modells enthält
- Index Buffer: wird oft mit Vertex Buffer kombiniert - Index Buffer speichert die Reihenfolge, in der die Vertices gezeichnet werden sollen (spart Speicherplartz)
*/

#include <fantom/dataset.hpp>

#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/cells.hpp>
#include <vector>

#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <fantom-plugins/utils/Graphics/ObjectRenderer.hpp>

using namespace fantom;

namespace { // nur in dieser Datei sichtbare Klassen
    class GridDisplayAlgorithm : public VisAlgorithm { // erbt von VisAlgorithm (was auch von Algorithm kommt)
    public:
        struct Options : public VisAlgorithm::Options {
            Options(fantom::Options::Control& control)
                : VisAlgorithm::Options(control) {
                add<PointSetBase>("Grid", "anzuzeigendes Grid"); // 3D-Gitter, das visualisiert werden soll
                add<Color>("Color", "Farbe für die Gitterlinien oder -flächen", Color(0.0, 1.0, 0.0));
                add<int>("Cell Index", "Index der anzuzeigenden Zelle (-1 für alle Zellen)", -1); // Index spezifischer Zelle, die visualisiert werden soll; -1 heißt alle Zellen anzeigen
                add<std::string>("Display Mode", "Wähle 'Lines' für Gitterlinien oder 'Faces' für Zellenflächen", "Lines"); // Darstellungs-Modus: Gitterlinien oder Flächen der Zellen
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs { // Ausgabeparameter für Algorithmus
            VisOutputs(fantom::VisOutputs::Control& control)
                : VisAlgorithm::VisOutputs(control) {
                addGraphics("Grid Visualization"); // grafische Ausgabe als "Grid Visualization"
            }
        };

        GridDisplayAlgorithm(InitData& data) // Konstruktor: Initialisierung der Basisklasse VisAlgorithm mit übergebenen InitData (fantom::Algorithm::InitData Class Reference)
            : VisAlgorithm(data) {}

        virtual void execute(const Algorithm::Options& options, const volatile bool& /*abortFlag*/) override {
            auto base = options.get<const PointSetBase>("Grid"); // Auswahl des Gitters  wird ausgelesen // auto grid = options.get<std::shared_ptr<const Grid<3>>>("Grid"); // Auswahl der Eigenschaften wird ausgelesen

            if (!base) { // Prüfen der Gitterdaten -> wenn kein Gitter übergeben, dann Abbruch
                debugLog() << "Kein gültiges Gitter bereitgestellt" << std::endl;
                return;
            }
            std::cout << "base valid" << std::endl;
            auto grid = std::dynamic_pointer_cast<const Grid<3>>(base);
            auto color = options.get<Color>("Color");
            int cellIndex = options.get<int>("Cell Index");
            std::string displayMode = options.get<std::string>("Display Mode");
            std::cout << __LINE__ << std::endl;
            std::vector<VectorF<3>> vertices; // Initialisierung Eckpunkte des Gitters
            std::vector<unsigned int> indices; // Initialisierung Reihenfolge der Eckpunkte, um Linien oder Flächen zu bilden

            if (displayMode == "Lines") { // Darstellungsmodus: Linien
                if (cellIndex == -1) { //
                    // alle Zellen des Gitters anzeigen (Linien)
                    for (size_t i = 0; i < grid->numCells(); ++i) {
                        addCellLines(grid, grid->cell(i), vertices, indices); // Linien jeder Zelle hinzugefügt
                    }
                } else {
                    if (cellIndex >= 0 && cellIndex < static_cast<int>(grid->numCells())) { // spezifischer Zellindex: nur angegebene Zelle verarbeitet
                        addCellLines(grid, grid->cell(cellIndex), vertices, indices);
                    } else {
                        debugLog() << "ungültiger Zellenindex" << std::endl;
                        return;
                    }
                }
            } else if (displayMode == "Faces") { // Darstellungsmodus: Flächen
                if (cellIndex == -1) {
                    // Alle Zellen des Gitters als Flächen anzeigen
                    for (size_t i = 0; i < grid->numCells(); ++i) {
                        addCellFaces(grid, grid->cell(i), vertices, indices); // Flächen der Gitterzellen hinzugefügt
                    }
                } else {
                    if (cellIndex >= 0 && cellIndex < static_cast<int>(grid->numCells())) {
                        addCellFaces(grid, grid->cell(cellIndex), vertices, indices);
                    } else {
                        debugLog() << "ungültiger Zellenindex" << std::endl;
                        return;
                    }
                }
            }

            if (vertices.empty()) {
                debugLog() << "keine Vertices zum Zeichnen" << std::endl; // Abbruch, da keine Vertecies generiert
                return;
            }

            auto const& system = graphics::GraphicsSystem::instance(); // Zugriff auf grafische Subsystem für GPU-Rendering
            std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath("utils/Graphics"); // Pfad zu shader-Dateien

            // Manuelle Berechnung der Bounding Sphere
            //auto boundingSphere = calculateBoundingSphere(vertices); //nimmt Liste der Vertices und berechnet Sphere, die alle Punkte umschließt
            auto boundingSphere = graphics::computeBoundingSphere( vertices );
            // Erstellen des Drawables
            std::shared_ptr<graphics::Drawable> drawable = system.makePrimitive( // Drawable-Objekt repräsentiert renderbares Objekt in der Szene
                graphics::PrimitiveConfig{ displayMode == "Lines" ? graphics::RenderPrimitives::LINES : graphics::RenderPrimitives::TRIANGLES } // LINES und TRIANGLES teilen Renderung der Primitivtyp: Linien und Dreiecke (3D) ->unterschiedliche Darstellungen desselben Modells möglich
                    .vertexBuffer("in_vertex", system.makeBuffer(vertices)) // in_vertex als Eingangsparameter für den Vertex Shader -> greift auf Vertex-Daten zu, um Transformationen und Berechnungen durchzuführen
                    .indexBuffer( system.makeIndexBuffer( indices ) )
                    .uniform("u_color", color) // Wert der bei Draw-Aufruf gleich bleibt (hier Farbe (color) an Shader übergeben)
                    .boundingSphere(boundingSphere), // Bounding Sphere wird Drawable-Objekt hinzugefügt (Sichtbarkeitsprüfungen)

                system.makeProgramFromFiles( // Shader-Programm
                    resourcePath + "shader/line/noShading/singleColor/vertex.glsl",   // Vertex Shader: definiert, wie Eckpunkte transformiert werden (wie Projektion von 3D in 2D)
                    resourcePath + "shader/line/noShading/singleColor/fragment.glsl") // Fragment Shader: bestimmt  Farbe jedes Pixels
                    //resourcePath + "shader/line/noShading/singleColor/geometry.glsl") // Geometry Shader: verarbeitet primitive Formen (Linien und Dreiecke) und kann neue Geometrien generieren
                                                                                 // TODO Erstmal für Lines
            );
            auto testObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            std::cout << "Drawable finished" << std::endl;
            // Drawable als Ausgabe setzen
            //setGraphics("Grid Visualization", drawable); // Hinzufügen der erstellten Grafik in Szene
            setGraphics("Grid Visualization",
                                     graphics::makeCompound( { drawable, testObjectRenderer->commit() } ) );
        }

    private:
        void addCellLines(std::shared_ptr<const Grid<3>> grid, const Cell& cell,
                          std::vector<VectorF<3>>& vertices, std::vector<unsigned int>& indices) { // Liniengeometrie für einzelne Gitterzelle
            size_t offset = vertices.size(); // aktuelles Offset: Startposition der neuen Vertices
            const ValueArray<Point3>& points = grid->points();

            for (size_t i = 0; i < cell.numVertices(); ++i) { // Hinzufügen der Eckpunkte
                vertices.push_back(VectorF<3>(points[cell.index(i)])); // Koordinaten jedes Eckpunkts der Zelle aus dem Gitter extrahiert und in vertices gespeichert
            }

            for (size_t i = 0; i < cell.numVertices(); ++i) { // Definieren der Linien
                indices.push_back(offset + i); // Linie wird durch Startpunkt geführt
                indices.push_back(offset + (i + 1) % cell.numVertices()); // Linie wird durch Endpunkt geführt (% um letzte Linie zurück zum ersten Punkt verbinden)
            }
        }

        void addCellFaces(std::shared_ptr<const Grid<3>> grid, const Cell& cell,
                          std::vector<VectorF<3>>& vertices, std::vector<unsigned int>& indices) { // Flächengeometrie (Triangulierung) für einzelne Gitterzelle
            size_t offset = vertices.size(); // Speichern Offset
            const ValueArray<Point3>& points = grid->points();

            for (size_t i = 0; i < cell.numVertices(); ++i) { // Hinzufügen der Eckpunkte
                vertices.push_back(VectorF<3>(points[cell.index(i)]));
            }

            for (size_t i = 0; i < cell.numVertices() - 2; ++i) { // Triangulieren der Fläche (für quadratische Zellen z.B. als 2 Dreiecke) -> um Fläche in Dreiecke zerlegen: erste Punkt (offset) als gemeinsamer Punkt -> anderen Punkte (offset + i + 1 und offset + i + 2) bilden jeweils ein Dreieck -> Viereck kann in 2 Dreiecke aufgeteilt werden
                indices.push_back(offset); // gemeinsamer Dreieck-Punkt
                indices.push_back(offset + i + 1); // 2. Dreieck-Punkt
                indices.push_back(offset + i + 2); // 3. Dreieck-Punkt
            }
        }

        graphics::BoundingSphere calculateBoundingSphere(const std::vector<VectorF<3>>& vertices) { // Berechnung Bounding Sphere
            VectorF<3> center(0.0f, 0.0f, 0.0f); // Berechnung des Schwerpukts (Center) der Bounding Sphere
            for (const auto& vertex : vertices) {
                center += vertex;
            }
            center /= static_cast<float>(vertices.size());

            float maxRadiusSquared = 0.0f; //  Berechnung des maximalen Radius -> Abstand zum center
            for (const auto& vertex : vertices) {
                VectorF<3> diff = vertex - center;
                float distanceSquared = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
                if (distanceSquared > maxRadiusSquared) {
                    maxRadiusSquared = distanceSquared;
                }
            }

            return graphics::BoundingSphere(center, std::sqrt(maxRadiusSquared));
        }

    };

    AlgorithmRegister<GridDisplayAlgorithm> reg("Grundaufgaben/2_DisplayGrid", "Zeigt ein Gitter an (Linien oder Flächen)");
} // namespace


#if 0

#include <fantom/algorithm.hpp>
#include <fantom/dataset.hpp>
#include <fantom/register.hpp>
#include <math.h>

#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <fantom-plugins/utils/Graphics/ObjectRenderer.hpp>

#include <stdexcept>
#include <vector>

using namespace fantom;

namespace
{

    class GridVis : public VisAlgorithm
    {

    public:
        struct Options : public VisAlgorithm::Options
        {
            Options( fantom::Options::Control& control )
                : VisAlgorithm::Options( control )
            {
                add< Color >( "Color", "Farbe der Ausgabe.", Color( 0.75, 0.75, 0.0 ) );
                add< bool >( "True/False", "True wenn die Seitenflächen, False wenn die Linien des Grids gezeichnet werden sollen.", true);
                add< Grid< 3 > >("Grid", "Das Input-Grid (3D).");
                add < int >("Cell_Index","Index der zu visualisierenden Zelle. Bei -1 wird das gesamte Grid gezeichnet", -1);
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs
        {
            VisOutputs( fantom::VisOutputs::Control& control )
                : VisAlgorithm::VisOutputs( control )
            {
                addGraphics( "Grid" );
            }
        };

        GridVis( InitData& data )
            : VisAlgorithm( data )
        {
        }

        //draw ist eine rekursive Funktion, die für das darstellen des grids verantwortlich ist
        virtual void draw( const Cell& zelle , const ValueArray< Point3 >& points, const Color color, const std::string resourcePath, const graphics::GraphicsSystem& system, std::vector< std::shared_ptr< graphics::Drawable > >& drawables, const bool b){
            //ist die Zelle ein Tetraeder, wird die Funktion rekursiv für alle Subzellen der Zelle aufgerufen
            if(zelle.type()== Cell::Type(4)){
                for(size_t i = 0; i < zelle.numFaces(); i++){
                    draw(zelle.face(i),points,color,resourcePath,system,drawables,b);
                }
            //ist die Zelle ein Würfel, wird die Funktion rekursiv für alle Subzellen der Zelle aufgerufen
            }else if(zelle.type()== Cell::Type(7)){
                for(size_t i = 0; i < zelle.numFaces(); i++){
                    draw(zelle.face(i),points,color,resourcePath,system,drawables,b);
                }
            //ist die Zelle eine Pyramide, wird die Funktion rekursiv für alle Subzellen der Zelle aufgerufen
            }else if(zelle.type()== Cell::Type(5)){
                for(size_t i = 0; i < zelle.numFaces(); i++){
                    draw(zelle.face(i),points,color,resourcePath,system,drawables,b);
                }
            //ist die Zelle ein Dreieck, so haben wir einen Zelltypen der gezeichnet werden kann, wenn der Input-Parameter "True/False" auf True gesetzt ist.
            }else if(zelle.type()== Cell::Type(2)){
                //ist "True/False" auf True, so zeichnen wir das Dreieck (wird gebraucht, um die Seitenflächen zu visualisieren)
                if(b){
                    //Erst werden alle Eckpunkte des Dreiecks extrahiert
                    std::vector< PointF< 3 > > tri( 3 );
                    tri[0] = PointF< 3 >(points[zelle.index(0)]);
                    tri[1] = PointF< 3 >(points[zelle.index(1)]);
                    tri[2] = PointF< 3 >(points[zelle.index(2)]);

                    //Es wird eine Bounding Sphere bestimmt, in der alle der drei Punkte liegen
                    auto bs = graphics::computeBoundingSphere(tri);

                    //anschließend wird das Dreieck als Primitive in drawables gespeichert
                    drawables.push_back(system.makePrimitive(
                        graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                        .vertexBuffer( "position", system.makeBuffer(tri) )
                        .uniform( "color", color )
                        .boundingSphere( bs ),
                    system.makeProgramFromFiles( resourcePath + "shader/surface/noShading/singleColor/vertex.glsl",
                                                 resourcePath + "shader/surface/noShading/singleColor/fragment.glsl" ) ));
                // ist "True/False" auf False, so wird die Funktion für alle Subzellen des Dreiecks aufgerufen
                }else if(!b){
                    for(size_t i = 0; i < zelle.numFaces(); i++){
                        draw(zelle.face(i),points,color,resourcePath,system,drawables,b);
                    }
                }
            //ist die Zelle ein Viereck, so haben wir einen Zelltypen der gezeichnet werden kann, wenn der Input-Parameter "True/False" auf True gesetzt ist.
            }else if(zelle.type()== fantom::Cell::Type(3)){
                //ist "True/False" auf True, so zeichnen wir das Viereck (es wird hierzu in 2 Dreiecke aufgeteilt, die im Anschluss gezeichnet werden)
                if(b){
                    //Das erste Dreieck wird extrahiert
                    std::vector< PointF< 3 > > tri1( 3 );
                    tri1[0] = PointF< 3 >(points[zelle.index(0)]);
                    tri1[1] = PointF< 3 >(points[zelle.index(1)]);
                    tri1[2] = PointF< 3 >(points[zelle.index(2)]);

                    //Das zweite Dreiecas Dreieckk wird extrahiert
                    std::vector< PointF< 3 > > tri2( 3 );
                    tri2[0] = PointF< 3 >(points[zelle.index(0)]);
                    tri2[1] = PointF< 3 >(points[zelle.index(2)]);
                    tri2[2] = PointF< 3 >(points[zelle.index(3)]);

                    //Beide dreiecke werden in einem gemeinsamen Vektor gespeichert
                    std::vector< PointF< 3 > > x[2];
                    x[0] = tri1;
                    x[1] = tri2;

                    for(int i = 0; i < 2; i++){
                        //Es wird eine Bounding Sphere bestimmt, in der alle der drei Punkte liegen
                        auto bs = graphics::computeBoundingSphere(x[i]);

                        //anschließend wird das Dreieck als Primitive in drawables gespeichert
                        drawables.push_back(system.makePrimitive(
                            graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                            .vertexBuffer( "position", system.makeBuffer(x[i]) )
                            .uniform( "color", color )
                            .boundingSphere( bs ),
                        system.makeProgramFromFiles( resourcePath + "shader/surface/noShading/singleColor/vertex.glsl",
                                                     resourcePath + "shader/surface/noShading/singleColor/fragment.glsl" ) ));
                    }

                // ist "True/False" auf False, so wird die Funktion für alle Subzellen des Dreiecks aufgerufen
                }else if(!b){
                    for(size_t i = 0; i < zelle.numFaces(); i++){
                        draw(zelle.face(i),points,color,resourcePath,system,drawables,b);
                    }
                }
            //Dieser Fall, der Fall das eine Zelle eine Linie ist, tritt nur in dem Fall ein, dass "True/False" auf False ist
            }else if(zelle.type()== fantom::Cell::Type(1)){
                //das if stellt sicher, dass hier auch kein Fehler auftritt
                if(!b){
                    //auch hier werden die beiden Endpunkte der Linie extrahiert
                    std::vector< PointF< 3 > > p(2);
                    p[0] = PointF< 3 >(points[zelle.index(0)]);
                    p[1] = PointF< 3 >(points[zelle.index(1)]);

                    //Es wird eine Bounding Sphere bestimmt, in der die zwei Punkte liegen
                    auto bs = graphics::computeBoundingSphere(p);

                    //anschließend wird die Linie als Primitive in drawables gespeichert
                    drawables.push_back(system.makePrimitive(
                        graphics::PrimitiveConfig{ graphics::RenderPrimitives::LINES }
                        .vertexBuffer( "position", system.makeBuffer(p) )
                        .uniform( "color", color )
                        .boundingSphere( bs ),
                    system.makeProgramFromFiles( resourcePath + "shader/surface/noShading/singleColor/vertex.glsl",
                                                 resourcePath + "shader/surface/noShading/singleColor/fragment.glsl" ) ));
                }
            }
        }

        virtual void execute( const Algorithm::Options& options, const volatile bool& abortFlag) override
        {
            /*
            Hier werden die Input-Parameter in Parametern color, b, c und grid gespeichert
            */
            Color color = options.get< Color >("Color");
            bool b = options.get< bool >("True/False");
            int c = options.get<int>("Cell_Index");
            std::shared_ptr< const Grid< 3 > > g = options.get<Grid<3>>("Grid");
            const ValueArray< Point3 >& points = g->points();

            //Eine Instanz von GraphicsSystem wird erstellt, um drawables später grafisch ausgeben zu können
            auto const& system = graphics::GraphicsSystem::instance();
            //Das ist der Dateipfad, in dem die Tools zu finden sind, mit welchen man die Primitives visualisieren kann
            std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );

            std::vector< std::shared_ptr< graphics::Drawable > > drawables;

            //Hier findet die Abfrage statt, ob man das ganze Grid oder nur einen einzelnen Index visualisieren möchte
            if(c==-1){
                for(size_t i = 0; i < g->numCells();i++){
                    draw(g->cell(i),points,color, resourcePath, system, drawables,b);
                }
            }else if(c>=g->numCells()||c < -1){
                debugLog() << "Der angegebene Zell-Index existiert nicht." << std::endl;
                return;
            }else{
                draw(g->cell(c),points,color, resourcePath, system, drawables,b);
            }

            setGraphics("Grid", graphics::makeCompound( drawables ));
        }
    };
    AlgorithmRegister< GridVis > dummy("Grundaufgaben/A2", "Visualisierungsalgorithmus für ein gegebenes Gitter.");
}

#endif
