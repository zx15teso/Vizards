#if 0

/*Aufgabe 1.2 Unstrukturiertes Gitter
Im Gegensatz zu strukturierten Gittern muss bei einem unstrukturierten Gitter eigenst ̈andig
beschrieben werden, wie das Gitter genau aufgebaut sein soll. F ̈ur diesen Teil soll ein zu-
sammenh ̈angendes, unstrukturiertes Gitter mit mindestens drei verschiedenen Zelltypen
erzeugt werden. Stellen Sie sicher, dass Ihr Algorithmus f ̈ur aneinander angrenzende Zellen
keine Punkte dupliziert, sondern bestehende Punkte wiederverwendet. Um diese Aufgabe
zu l ̈osen lohnt sich ein Blick in das Tutorial Fields, Grids, and Cells insbesondere der
dort als erstes aufgelisteten Klasse fantom::Cell, da sind die unterschiedlichen Zelltypen
beschrieben, die in FAnToM bereits implementiert sind. Um ausgehend von Ihrer Be-
schreibung von Zellen ein zusammenh ̈angendes Gitter erzeugen zu k ̈onnen, sollten Sie in
der Dokumentation nach DomainFactory suchen und dort die Funktion makeGrid lokali-
sieren.
Zum  ̈Uberpr ̈ufen Ihrer Ergebnisse k ̈onnen Sie das Grid/Show Grid -Algorithmus oder
das in der 2. Aufgabe zu erstellende Plugin verwenden.

- Ziel:  zusammenhängendes Gitter mit mindestens drei verschiedenen Zelltypen erzeugen lassen, ohne Duplikate von Punkten für benachbarte Zellen
- bestehende Gitterpunkte wiederverwenden

*/

#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/dataset.hpp>
#include <fantom/cells.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/datastructures/DomainFactory.hpp>


using namespace fantom;

namespace
{
    class UnstructuredGridAlgorithm : public VisAlgorithm
    {
    public:
        struct Options : public VisAlgorithm::Options
        {
            Options(fantom::Options::Control& control)
                : VisAlgorithm::Options(control)
            {
                add< double >( "GridSize", "The size of the grid", 10.0 );
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs
        {
            VisOutputs(fantom::VisOutputs::Control& control)
                : VisAlgorithm::VisOutputs(control)
            {
                addGraphics( "Grid" );
            }
        };

        UnstructuredGridAlgorithm( InitData& data )
            : VisAlgorithm( data )
        {
        }

        virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/) override
        {
            double gridSize = options.get< double >( "GridSize" );

            // Erstellen der Punktliste und Zellen
            std::vector< Cell::Type > cellTypes;
            std::vector< Point3 > points;               // Punkte im Gitter
            std::vector< std::vector< size_t > > cells; // Liste der Zellen

            // Punkte erstellen
            points.push_back(Point3(0, 0, 0));                        // Punkt 0
            points.push_back(Point3(gridSize, 0, 0));                 // Punkt 1
            points.push_back(Point3(0, gridSize, 0));                 // Punkt 2
            points.push_back(Point3(0, 0, gridSize));                 // Punkt 3
            points.push_back(Point3(gridSize, gridSize, gridSize));   // Punkt 4

            // Zelltypen definieren
            cells.push_back({0, 1});                                  // Kante (EDGE)
            cellTypes.push_back(Cell::Type::EDGE);

            cells.push_back({0, 1, 2});                               // Dreieck (TRIANGLE)
            cellTypes.push_back(Cell::Type::TRIANGLE);

            cells.push_back({0, 1, 2, 3});                            // Tetraeder (TETRAHEDRON)
            cellTypes.push_back(Cell::Type::TETRAHEDRON);

            // Gitter erstellen
            auto grid = DomainFactory::makeGrid(points, cells, cellTypes, nullptr);

            // Visualisierung vorbereiten
            std::shared_ptr< const Grid< 3 > > grid3D = grid;
            const ValueArray< Point3 >& gridPoints = grid3D->points();

            // Punkte in das Visualisierungssystem einfügen
            std::vector< VectorF< 3 > > vertices;
            for (size_t i = 0; i < gridPoints.size(); ++i) {
                vertices.push_back(VectorF< 3 >(gridPoints[i]));
            }

            auto const& system = graphics::GraphicsSystem::instance();
            auto glyphs = system.makePrimitive(
                graphics::PrimitiveConfig{ graphics::RenderPrimitives::POINTS }
                    .vertexBuffer("in_vertex", system.makeBuffer(vertices))
                    .uniform("u_color", Color(0.75, 0.75, 0.0))
                    .uniform("u_pointSize", 5.0f)
            );

            setGraphics("Grid", glyphs);
        }
    };

    AlgorithmRegister<UnstructuredGridAlgorithm> registerGenerateGridData("Grundaufgaben/1.2_GenerateUnstructuredGridData", "Erzeugt unstrukturiertes Gitter 1.2");

} // namespace

#endif
