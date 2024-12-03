/* Aufgabe 1: Erzeugen von Gittern
Ihre erste praktische  ̈Ubung mit FAnToM ist das Erzeugen einer Gitterstruktur als Zell-
komplex. Dabei soll ein Algorithmus geschrieben werden, bei dem zwei Ausgaben generiert
werden sollen: ein strukturiertes Gitter und ein unstrukturiertes Gitter. Es soll dabei kein
DataObjectBundle genutzt werden! Sie k ̈onnen die Position der Gitterpunkte in dieser
Aufgabe frei w ̈ahlen, der Einfachheit halber wird empfohlen im Bereich -10 bis 10 zu
bleiben.


Aufgabe 1.1 Strukturiertes (uniformes) Gitter
Fur die Erzeugung eines strukturierten Gitters schauen Sie sich bitte das Tutorial Com-
mitting Field Data an.  ̈Ahnlich zum Tutorial sollen hier Eingabeoptionen genutzt werden,
um den Ursprung, die Ausdehnung und das Zellmaß dem Nutzer zu  uberlassen. Fur die
Eingabeoptionen schauen Sie sich bitte das Tutorial Algorithm Options an. */


#include <fantom/algorithm.hpp>
#include <fantom/dataset.hpp>
#include <fantom/register.hpp>

using namespace fantom;

namespace
{
    class GenerateGridDataAlgorithm : public DataAlgorithm // erbt von Basisklasse DataAlgorithm und implementiert Methode execute (z. 59), die Algorithmus ausführt
    {
    public:
        struct Options : public DataAlgorithm::Options // Eingabeparameter für Alg
        {
            Options(fantom::Options::Control& control)
                : DataAlgorithm::Options(control)
            {
                // Anzahl Gitterpunkte in jeweiligen Achsen X, Y, Z
                add<long>("nPointsX", "Anzahl der Punkte in X-Richtung", 10);
                add<long>("nPointsY", "Anzahl der Punkte in Y-Richtung", 10);
                add<long>("nPointsZ", "Anzahl der Punkte in Z-Richtung", 10);

                // minimalen und maximalen Werte für jede Dimension des Gitters
                add<double>("minX", "Minimalwert in X-Richtung", -10.0);
                add<double>("maxX", "Maximalwert in X-Richtung", 10.0);
                add<double>("minY", "Minimalwert in Y-Richtung", -10.0);
                add<double>("maxY", "Maximalwert in Y-Richtung", 10.0);
                add<double>("minZ", "Minimalwert in Z-Richtung", -10.0);
                add<double>("maxZ", "Maximalwert in Z-Richtung", 10.0);
            }
        };

        struct DataOutputs : public DataAlgorithm::DataOutputs // Definition Algo-Ergebnis: Funktion mit Punkte des Gitters: grid
        {
            DataOutputs(fantom::DataOutputs::Control& control)
                : DataAlgorithm::DataOutputs(control)
            {
                add<Grid<3>>("grid");
            }
        };

        GenerateGridDataAlgorithm(InitData& data)
            : DataAlgorithm(data)
        {
        }

        virtual void execute(const Algorithm::Options& options, const volatile bool& /*abortFlag*/) override
        {
            // Optionen für das Gitter holen
            long nPointsX = options.get<long>("nPointsX");
            long nPointsY = options.get<long>("nPointsY");
            long nPointsZ = options.get<long>("nPointsZ");
            double minX = options.get<double>("minX");
            double maxX = options.get<double>("maxX");
            double minY = options.get<double>("minY");
            double maxY = options.get<double>("maxY");
            double minZ = options.get<double>("minZ");
            double maxZ = options.get<double>("maxZ");

            // Anzahl der Punkte pro Achse und der Schrittweiten berechnen
            double spacingX = (maxX - minX) / (nPointsX - 1);
            double spacingY = (maxY - minY) / (nPointsY - 1);
            double spacingZ = (maxZ - minZ) / (nPointsZ - 1);

            // Gitterabmessungen und Ursprungswerte für das Gitter
            size_t extent[] = {static_cast<size_t>(nPointsX), static_cast<size_t>(nPointsY), static_cast<size_t>(nPointsZ)};
            double origin[] = {minX, minY, minZ};
            double spacing[] = {spacingX, spacingY, spacingZ};

            // Gitter erstellen
            auto grid = DomainFactory::makeUniformGrid(extent, origin, spacing); //nimmt Anzahl der Punkte in jeder Dimension, Ursprung und Abstände zwischen Punkten als Eingabeparameter

            // Ausgabe der Gitterdaten als Funktion auf dem Gitter; für jedes Gitterpunkt-Paar (i, j, k) wird ein Punkt Vector(x, y, z) erzeugt und zur Liste points hinzugefügt
            std::vector<Vector3> points;
            for (long i = 0; i < nPointsX; ++i)
            {
                for (long j = 0; j < nPointsY; ++j)
                {
                    for (long k = 0; k < nPointsZ; ++k)
                    {
                        double x = minX + i * spacingX;
                        double y = minY + j * spacingY;
                        double z = minZ + k * spacingZ;
                        points.push_back(Vector3(x, y, z)); //für jedes Gitterfeld (X, Y, Z) wird ein Punkt erzeugt und zur points-Liste hinzugefügt; Punkt hat Form Vector3, der Koordinaten in 3D speichert
                    }
                }
            }

            // Registriere die Punkte auf dem Gitter
            std::shared_ptr<const Function<Vector3>> function = addData(grid, fantom::PointSetBase::Points, points); // nach Berechnung aller Punkte wird Gitter als Function<Vector3> gespeichert und in System eingefügt; addData registriert Gitter als Datensatz

            // Setze das Ergebnis der Algorithmus-Ausgabe
            setResult("grid", grid); // Gitter als grid in FAnToM
        }
    };

    AlgorithmRegister<GenerateGridDataAlgorithm> registerGenerateGridData("Grundaufgaben/1.1_GenerateStructuredGridData", "Erzeugt strukturiertes Gitter mit Punkten aus 1.1");
}
