# if 0
/* Aufgabe 4 - Integrationen von Stromlinien
Integrationslinien geh¨oren zu den elementaren Visualisierungsprimitiven bei der Analyse
von Vektorfeldern. Fur zeitunabh ¨ ¨angige Vektorfelder berechnet man Stromlinien. Zur Berechnung gibt es viele Methoden, die sich in Genauigkeit und erforderlicher Rechenleistung
unterscheiden.
• Implementieren Sie eine DataAlgorithm Klasse Integrator. Als zwei Ableitungen
dieser Klasse implementieren Sie das Euler-Verfahren mit adaptiver Schrittweitensteuerung (nicht das modifizierte Euler-Verfahren!) und danach das Runge-KuttaVerfahren 4. Ordnung (hier reicht eine feste Schrittweite). Die Formeln fur beide Ver- ¨
fahren befinden sich im Skript zur Vorlesung (oder im Netz). Hilfe bei der Implementierung bietet das Tutorial zu Fields, Grids, and Cells, welches die Benutzung
der Evaluator-Klasse zum Zweck der Interpolation demonstriert. Beide Verfahren
sollen die Stromlinien nur innerhalb der Domain bzw. des Gitters integrieren.
• Verwenden Sie den Integrator zur Erzeugung von Stromlinien (3D ist ausreichend,
2D optional). Die Ausgabe erfolgt uber Liniensegmente (in FAnToM das sogenan- ¨
nte LineSet). Testen Sie ihn an den verschiedenen Vektorfeld-Datens¨atzen, nutzen Sie zur Darstellung des LineSets den bereits vorhandenen Show Line SetAlgorithmus. Zum uberpr ¨ ufen der Ergebnisse kann das ¨ Show Hedgehogs-Algorithmus
zur Darstellung des Vektor- feldes herangezogen werden. Zur Uberpr ¨ ufung der Funk- ¨
tionalit¨at Ihres Algorithmus k¨onnen Sie die zwei Datens¨atze streamTest1.vtk und
streamTest2.vtk nutzen.
HINWEIS: Es ist in den seltensten F¨allen praktikabel an jeder Position im Datensatz eine
Stromiline zu starten. Uberlegen Sie sich daher sinnvolle Alternativen (zum Beispiel eine ¨
Linie oder Fl¨ache auf der ¨aquidistant einige Stromlinien gestartet werden) und stellen Sie
dem Nutzer dafur in ihrem Plugin entsprechende Optionen bereit.

Wissen:
1. Euler-Verfahren mit adaptiver Schrittweitensteuerung: Beim Euler-Verfahren handelt es sich um eine einfache Methode zur Berechnung von Stromlinien, bei der die Position eines Punktes im Vektorfeld durch die Geschwindigkeit (also den Vektor im Feld) über einen kleinen Zeitschritt aktualisiert wird.
   Um adaptiv zu sein, wird die Schrittweite je nach Fehler in der Berechnung angepasst.
2. Runge-Kutta-Verfahren (4. Ordnung): Das Runge-Kutta-Verfahren 4. Ordnung ist genauer als das Euler-Verfahren, weil es mehrere Berechnungen innerhalb eines einzelnen Zeitschritts verwendet und dann eine gewichtete Summe der Ergebnisse zurückgibt.
*/

#include <fantom/algorithm.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
#include <vector>

using namespace fantom;

namespace {

    // Integrator-Klasse, die von VisAlgorithm erbt, um Visualisierungsausgaben zu integrieren
    class Integrator : public VisAlgorithm {
    public:
        struct Options : public VisAlgorithm::Options {
            Options(fantom::Options::Control& control)
                : VisAlgorithm::Options(control) {
                add<Field<3, Vector3>>("Field", "A 3D vector field", definedOn<Grid<3>>(Grid<3>::Points));
                add<double>("Time", "The time when to sample the field.", 0.0);
                add<double>("StepSize", "The step size for integration.", 0.01);
                add<int>("MaxSteps", "Maximum number of steps for each line.", 1000);
            }
        };

        struct VisOutputs : public VisAlgorithm::VisOutputs {
            VisOutputs(fantom::VisOutputs::Control& control)
                : VisAlgorithm::VisOutputs(control) {
                // Hier wird die grafische Ausgabe für die Stromlinien hinzugefügt
                addGraphics("Streamlines");
            }
        };

        Integrator(InitData& data) : VisAlgorithm(data) {}

        virtual void execute(const Algorithm::Options& options, const volatile bool& abortFlag) override {
            // Holt das Vektorfeld und die Integrationsparameter
            auto field = options.get<Field<3, Vector3>>("Field");
            double time = options.get<double>("Time");
            double stepSize = options.get<double>("StepSize");
            int maxSteps = options.get<int>("MaxSteps");

            if (!field) {
                debugLog() << "Input Field not set." << std::endl;
                return;
            }

            // Initialisiert den Evaluator für das Vektorfeld
            auto evaluator = field->makeEvaluator();
            std::shared_ptr<const Grid<3>> grid = field->domain();
            const ValueArray<Point3>& points = grid->points();

            // Speicher für die berechneten Stromlinien
            std::vector<VectorF<3>> streamlines;

            // Integration durch das Gitter
            for (size_t i = 0; i < points.size(); ++i) {
                Point3 startPoint = points[i];
                if (abortFlag) return;

                // Berechnet die Stromlinie ab dem Startpunkt
                std::vector<Point3> streamline;
                streamline.push_back(startPoint);

                Point3 currentPoint = startPoint;
                for (int step = 0; step < maxSteps; ++step) {
                    if (abortFlag) return;

                    // Holt den Vektor an der aktuellen Position
                    evaluator->reset(currentPoint, time);
                    Vector3 velocity = evaluator->value();

                    // Euler-Verfahren für die Integration
                    currentPoint += velocity * stepSize;
                    streamline.push_back(currentPoint);

                    // Überprüft, ob der Punkt noch innerhalb des Gitters liegt
                    if (!grid->contains(currentPoint)) {
                        break;
                    }
                }

                // Wenn die Stromlinie gültig ist (mehr als ein Punkt)
                if (streamline.size() > 1) {
                    for (size_t j = 0; j < streamline.size() - 1; ++j) {
                        // Füge die Linie in das Streamlines-Array ein
                        streamlines.push_back(VectorF<3>(streamline[j]));
                        streamlines.push_back(VectorF<3>(streamline[j + 1]));
                    }
                }
            }

            // Wenn die Berechnung abgebrochen wurde, breche ab
            if (abortFlag) return;

            // Erstelle das Drawable für die Stromlinien
            auto const& system = graphics::GraphicsSystem::instance();
            std::shared_ptr<graphics::Drawable> streamlinesDrawable = system.makePrimitive(
                graphics::PrimitiveConfig{ graphics::RenderPrimitives::LINES }
                    .vertexBuffer("in_vertex", system.makeBuffer(streamlines))
                    .uniform("u_lineWidth", 1.0f)
                    .uniform("u_color", Color(0.75, 0.75, 0.0)) // gelbe Farbe für die Stromlinien
                    .boundingSphere(graphics::computeBoundingSphere(streamlines))
            );

            // Setzt das Drawable als Ausgabe der Stromlinien
            setGraphics("Streamlines", streamlinesDrawable);
        }
    };

    AlgorithmRegister<Integrator> dummy("Tutorial/Integrator",
                                        "Generate streamlines using the Euler method with adaptive step size.");

} // namespace

# endif
