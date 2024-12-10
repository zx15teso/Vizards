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

namespace {
    class GenerateUnstructuredGrid : public DataAlgorithm {
    public:
        struct Options : public DataAlgorithm::Options {
            Options(fantom::Options::Control& control)
                : DataAlgorithm::Options(control) {
            }
        };

        struct Outputs : public DataAlgorithm::DataOutputs {
            Outputs(Control& control)
                : DataAlgorithm::DataOutputs(control) {
                // Die Ausgabe ist ein Gitter, das als DataObject verwendet werden kann
                add<std::shared_ptr<const Grid<3>>>("Grid");
            }
        };

        GenerateUnstructuredGrid(InitData& data)
            : DataAlgorithm(data) {}

        virtual void execute(const Algorithm::Options& /*options*/, const volatile bool& /*abortFlag*/) override {
            // 1. Punkte definieren (gemeinsame Punkte für die Zellen)
            std::vector<Point3> points = {
                Point3(0.0, 0.0, 0.0),  // Punkt 0
                Point3(1.0, 0.0, 0.0),  // Punkt 1
                Point3(0.5, 1.0, 0.0),  // Punkt 2
                Point3(0.0, 0.0, 1.0),  // Punkt 3
                Point3(1.0, 0.0, 1.0),  // Punkt 4
                Point3(0.5, 1.0, 1.0),  // Punkt 5
                Point3(-0.5, 0.5, 0.0)  // Punkt 6
            };

            // 2. Zellen definieren (Dreieck, Tetraeder und Prisma)
            std::vector<std::shared_ptr<Cell>> cells;
            cells.push_back(std::make_shared<Cell>(Cell::Type::TRIANGLE, std::vector<size_t>{0, 1, 2}));
            cells.push_back(std::make_shared<Cell>(Cell::Type::TETRAHEDRON, std::vector<size_t>{0, 1, 2, 3}));
            cells.push_back(std::make_shared<Cell>(Cell::Type::PRISM, std::vector<size_t>{0, 1, 2, 3, 4, 5}));
            cells.push_back(std::make_shared<Cell>(Cell::Type::QUAD, std::vector<size_t>{0, 1, 3, 4}));

            // 3. Zellanzahl und Zelltypen
            size_t numCellTypes = 4;
            std::pair<Cell::Type, size_t> cellCounts[] = {
                {Cell::Type::TRIANGLE, 1},
                {Cell::Type::TETRAHEDRON, 1},
                {Cell::Type::PRISM, 1},
                {Cell::Type::QUAD, 1}
            };

            // 4. Indices für die Punkte generieren
            std::vector<size_t> indices;
            for (const auto& cell : cells) {
                for (size_t i = 0; i < cell->numVertices(); ++i) {
                    indices.push_back(cell->index(i));
                }
            }

            // 5. Gitter mit der DomainFactory erstellen
            auto grid = DomainFactory::makeGrid(points, numCellTypes, cellCounts, indices);

            // 6. Ergebnis speichern
            setResult("Grid", grid);
        }
    };


    AlgorithmRegister<GenerateUnstructuredGrid> reg("Grundaufgaben/1.2 Generate Unstructured Grid", "Erzeugt ein unstrukturiertes Gitter mit mehreren Zelltypen.");
}

 #endif


#include <fantom/algorithm.hpp>
#include <fantom/dataset.hpp>
#include <fantom/register.hpp>

using namespace fantom;

namespace
    {

    class Gitter2 : public DataAlgorithm
    {

    public:
        struct Options : public DataAlgorithm::Options
        {
            Options( fantom::Options::Control& control )
                : DataAlgorithm::Options( control )
            {
                /*
                nx, ny, nz beschreiben die Ausdehnung des Grids in die drei Raumrichtungen x,y,z
                dx, dy, dz beschreiben den Abstand zwischen den Gitterpunkten auf den Koordinatenachsen
                */
                add< long >( "nx", "", 10 );
                add< long >( "ny", "", 10 );
                add< long >( "nz", "", 10 );

                addSeparator();
                add< double >( "dx", "", 1.0 );
                add< double >( "dy", "", 1.0 );
                add< double >( "dz", "", 1.0 );
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs
        {
            DataOutputs( fantom::DataOutputs::Control& control )
                : DataAlgorithm::DataOutputs( control)
            {
                //Ein Grid wird als DataOutput definiert
                add< const Grid< 3 > >( "grid" );
            }
        };


        Gitter2( InitData& data )
            : DataAlgorithm( data )
        {
        }

        virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override
        {
            //Das Array gibt an, aus welchen Punkten im Raum das Grid gebildet wird.
            const std::vector<fantom::Point3> points = {
                {0, 0, 0},  // 0
                {1, 0, 0},  // 1
                {0, 1, 0},  // 2
                {1, 1, 0},  // 3
                {0, 0, 1},  // 4
                {1, 0, 1},  // 5
                {0, 1, 1},  // 6
                {1, 1, 1},  //7
                {0.5, 0.5, -1},  //8
                {0, 1, 2} //9
            };
            //Diese Zahl gibt an, wie viele verschiedene Typen von Zellen es im Grid gibt.
            size_t numCellTypes = 3;

            //Dieses Array von Paaren definiert, welche Zelltypen es im konkreten gibt und in welcher Anzahl sie vorkommen.
            const std::pair< Cell::Type, size_t >    cellCounts[] = {
                {Cell::HEXAHEDRON, 1}, //ein Würfel
                {Cell::TETRAHEDRON, 1}, //ein Tetraeder
                {Cell::PYRAMID, 1} //eine Pyramide
            };

            //Mittels der Indizes der Punkte aus dem points-Array wird nun ein Vektor von Indizes bestimmt, der den Zellen Punkte im Raum zuordnet.
            std::vector<size_t> indices = {
                0, 1, 5, 4, 6, 7, 3, 2,   // Wuerfel
                4, 5, 6, 9,    // Tetraeder
                0, 1, 3, 2, 8 // Pyramide
            };

            std::shared_ptr<const Grid<3>> grid = DomainFactory::makeGrid<3>(points, numCellTypes, cellCounts, indices);

            setResult("grid", grid);
        }
    };
    AlgorithmRegister< Gitter2 > dummy1( "Grundaufgaben/A1T2", "Generierung eines 3D Grids mit selbst definierten Zellen." );
}
