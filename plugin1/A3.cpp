# if 0
/* Aufgabe 3 - Einfache Felddarstellung
 *
In dieser Aufgabe sollen Sie einen VisAlgorithm schreiben, der alle Positionen markiert,
die einen bestimmten Wert ubersteigen. Diese einfache Art der Visualisierung durch ¨
Ausw¨ahlen wird h¨aufig angewendet, um einen schnellen ersten Eindruck des Datensatzes zu bekommen. Als Markierung bieten sich hier am ehesten Kugeln oder Punkte an.
Der Algorithmus erwartet ein Skalarfeld und den Schwellwert als Eingabe. Bei dieser Aufgabe ist es n¨otig das Skalarfeld direkt an den Gridpunkten auszuwerten. Dazu ben¨otigen
Sie die Klasse DiscreteFunctionEvaluator, nicht den FunctionEvaluator. Letzteren
ben¨otigen Sie in den sp¨ateren Aufgaben!

Zur Uberpr ¨ ufung der Korrektheit k ¨ ¨onnen Sie den Algorithmus mit dem Datensatz
markPosScalar1.vtk testen. Bei einem Schwellwert von 8e−4 sollten Sie nun den (2,1,0)-
Elektronenorbit eines Wasserstoffatoms sehen (vorausgesetzt, Sie haben alles korrekt implementiert und wissen, wie dieser Orbit aussieht). Analog kann auch markPosScalar2.vtk
mit dem Schwellwert 2.4e − 3 getestet werden. Ein Beispiel finden Sie in Abbildung 5.
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#include <fantom/datastructures/Function.hpp>
#include <fantom/algorithm.hpp>
#include <fantom/dataset.hpp>
#include <fantom/register.hpp>

using namespace fantom;

namespace {

    //  DiscreteFunctionEvaluator als abstrakter Evaluator -> liefert Skalarwerte für Gitterpunkte und Zeitschritte
    template<typename T>
    class DiscreteFunctionEvaluator {
    public:
        // Konstruktor, der Skalarwerte für jedes Zeit-Segment initialisiert
        DiscreteFunctionEvaluator(const std::vector<std::vector<T>>& data)
            : data_(data), currentTimeStep_(0) {}

        // aktueller Zeitschritt (dass der Zeitschritt in gültigem Bereich liegt)
        void reset(size_t timeStep) {
            if (timeStep >= numTimeSteps()) {
                std::cerr << "ungültiger Zeitschritt!" << std::endl;
                return;
            }
            currentTimeStep_ = timeStep;
        }

        // Gibt den Skalarwert für den i-ten Punkt im aktuellen Zeitschritt zurück
        T value(size_t i) const {
            return data_[currentTimeStep_][i];
        }

        // Zugriff über den Index-Operator auf den Wert des Skalarfeldes
        T operator[](size_t i) const {
            return value(i);
        }

        // Gibt die Anzahl der Werte im aktuellen Zeitschritt zurück
        size_t numValues() const {
            return data_[currentTimeStep_].size();
        }

        // Gibt die Anzahl der Zeitschritte zurück
        size_t numTimeSteps() const {
            return data_.size();
        }

        // Gibt die Gesamtgröße des aktuellen Zeitschrittes zurück
        size_t size() const {
            return numValues();
        }

    private:
        std::vector<std::vector<T>> data_;  // 2D-Datenstruktur (Zeitschritte * Werte)
        size_t currentTimeStep_;  // Der aktuell eingestellte Zeitschritt
    };

} // namespace

// Funktion zur Visualisierung von Werten, die einen Schwellenwert überschreiten
void VisualizeScalarFieldAboveThreshold(
    const fantom::DiscreteFunctionEvaluator<double>& evaluator,  // Der Evaluator mit den Skalarwerten
    double threshold)  // Der Schwellenwert für die Markierung
{
    std::cout << "Markierte Punkte (Werte > " << threshold << "):" << std::endl;

    // Iteriere über alle Punkte und überprüfe, ob der Skalarwert über dem Schwellenwert liegt
    for (size_t i = 0; i < evaluator.numValues(); ++i) {
        double scalarValue = evaluator[i];  // Hole den Skalarwert für den Punkt

        // Wenn der Skalarwert den Schwellenwert überschreitet, markiere den Punkt
        if (scalarValue > threshold) {
            std::cout << "Punkt " << i << " hat den Wert " << scalarValue << std::endl;
        }
    }
}

int main() {
    // Beispielhafte Daten für die Skalarwerte (1 Zeitschritt, 5 Punkte)
    std::vector<std::vector<double>> exampleData = {{0.001, 0.005, 0.008, 0.003, 0.007}};  // 1 Zeitstempel, 5 Werte

    // Erstelle einen Evaluator mit den Beispielwerten
    fantom::DiscreteFunctionEvaluator<double> evaluator(exampleData);

    // Definiere einen Schwellenwert für die Visualisierung
    double threshold = 0.006;

    // Visualisiere das Skalarfeld, wobei nur Werte oberhalb des Schwellenwerts angezeigt werden
    VisualizeScalarFieldAboveThreshold(evaluator, threshold);

    return 0;
}

# endif

#include <fantom/algorithm.hpp>
#include <fantom/dataset.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
#include <math.h>

#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <fantom-plugins/utils/Graphics/ObjectRenderer.hpp>

#include <stdexcept>
#include <vector>

using namespace fantom;

namespace
{

    class ScalarfieldAlgorithm : public VisAlgorithm
    {

    public:
        struct Options : public VisAlgorithm::Options
        {
            Options( fantom::Options::Control& control )
                : VisAlgorithm::Options( control )
            {
                add< Function<Scalar> >( "Field", "Ein 3D-Skalarfeld.", definedOn< Grid< 3 > >( Grid< 3 >::Points ) );
                add< Color >( "Color", "Die Farbe der anzuzeigenden Kugeln.", Color( 0.75, 0.75, 0.0 ) );
                add< double > ("Threshold", "Der Threshold, ab wann eine Kugel gezeichnet werden soll.", 0.0);
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs
        {
            VisOutputs( fantom::VisOutputs::Control& control )
                : VisAlgorithm::VisOutputs( control )
            {
                addGraphics( "Spheres" );
            }
        };

        ScalarfieldAlgorithm( InitData& data )
            : VisAlgorithm( data )
        {
        }

        virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override
        {
            /*
            Hier werden die Input-Parameter in Parametern cfield, threshold und color gespeichert
            */
            std::shared_ptr<const Function<Scalar>> field = options.get<Function<Scalar>>("Field");
            const double threshold = options.get<double>( "Threshold" );
            const Color color = options.get<Color>("Color");

            //Wenn kein oder ein falsches Feld ausgewählt wurde, soll eine Debug-Nachricht erscheinen
            if( !field )
            {
                debugLog() << "Es wurde kein Input-Feld ausgewählt." << std::endl;
                return;
            }

            std::shared_ptr< const Grid< 3 > > grid = std::dynamic_pointer_cast< const Grid< 3 > >( field->domain() );
            const ValueArray< Point3 >& points = grid->points();

            //Es wird eine Instanz von GraphicsSystem gebildet, um die Kugeln später grafisch ausgeben zu können
            auto const& system = graphics::GraphicsSystem::instance();
            //Es wird ein performanceObjectRenderer erstellt und ihm werden so viel Rechenleistung allokiert, wie es Punkte im Skalarfeld gibt
            auto performanceObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            performanceObjectRenderer->reserve( graphics::ObjectRenderer::ObjectType::SPHERE, grid->numPoints() );

            //es wird ein evaluator benötigt, um in der for-Schleife den Skalarwert des i-ten Punktes zu bestimmen
            auto eval = field->makeDiscreteEvaluator();

            for(size_t i = 0; i < grid->numPoints(); ++i )
            {
            auto v = eval->value(i);
                //ist der Wert des Skalars größer als der Threshold, so wird dem ObjectRenderer eine Sphere am jeweiligen Punkt mit der Größe 0.25 und der Farbe color hinzugefügt
                if(v[0] > threshold){
                    performanceObjectRenderer->addSphere( points[i], 0.25, color);
                }
            }
            setGraphics( "Spheres", performanceObjectRenderer->commit());
        }
    };
    AlgorithmRegister< ScalarfieldAlgorithm > dummy("Grundaufgaben/A3","Visualisiere alle Punkte eines Skalarfeldes, die einen bestimmten Threshold übersteigen.");
}
