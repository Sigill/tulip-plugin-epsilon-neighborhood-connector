#include <tulip/ImportModule.h>
#include <tulip/TulipPluginHeaders.h>
#include <stdexcept>
#include <vector>
#include <cmath>

#define CHECK_PROP_PROVIDED(PROP, STOR) \
	do { \
		if(!dataSet->get(PROP, STOR)) \
		throw std::runtime_error(std::string("No \"") + PROP + "\" property provided."); \
	} while(0)

using namespace std;
using namespace tlp;

#define DISTANCES "Euclidian;Manhattan;Chebychev"

namespace {
	const char * SupportedDistances = DISTANCES;
	const char * paramHelp[] = {
		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "LayoutProperty" ) \
			HTML_HELP_BODY() \
			"The layout on which the distance will be computed." \
			HTML_HELP_CLOSE(),

		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "DoubleProperty" ) \
			HTML_HELP_BODY() \
			"The property in which to store the computed distance." \
			HTML_HELP_CLOSE(),

		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "String" ) \
			HTML_HELP_DEF("Values", DISTANCES)
			HTML_HELP_DEF("Default", "Euclidian")
			HTML_HELP_BODY() \
			"The type of distance to use." \
			HTML_HELP_CLOSE(),

		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "double" ) \
			HTML_HELP_BODY() \
			"Distance above which two nodes will not be connected." \
			HTML_HELP_CLOSE()
	};
}

class EpsilonNeighborhoodConnectorOnLayout:public tlp::Algorithm {
private:
	tlp::LayoutProperty *layout;
	tlp::DoubleProperty *metric;
	tlp::StringCollection distance_type;
	double max_distance;

public:
	PLUGININFORMATIONS("Build epsilon-neighborhood on layout", "Cyrille FAUCHEUX", "2012-01-17", "", "1.0", "Topology Update")

	EpsilonNeighborhoodConnectorOnLayout(const tlp::PluginContext *context):Algorithm(context) {
		addInParameter< LayoutProperty >   ("layout",           paramHelp[0], "viewLayout");
		addInParameter< DoubleProperty >   ("metric",           paramHelp[1], "viewMetric");
		addInParameter< StringCollection > ("distance type",    paramHelp[2], SupportedDistances);
		addInParameter< double >           ("maximum distance", paramHelp[3], "1"         );
	}

	bool check(std::string &err) {
		try {
			if(dataSet == NULL)
				throw std::runtime_error("No dataset provided.");

			CHECK_PROP_PROVIDED("layout",           this->layout        );
			CHECK_PROP_PROVIDED("metric",           this->metric        );
			CHECK_PROP_PROVIDED("distance type",    this->distance_type );
			CHECK_PROP_PROVIDED("maximum distance", this->max_distance  );

			if(this->max_distance <= 0)
				throw std::runtime_error("The value for the \"maximum distance\" must be greater than 0.");
		} catch (std::runtime_error &ex) {
			err.assign(ex.what());
			return false;
		}

		return true;
	}

	bool run() {
		node u, v;
		edge e;
		Coord cu, cv;
		double d;

		std::vector< node > nodes;
		nodes.reserve(graph->numberOfNodes());

		{ // Building a list of the nodes, it will make iterating through it easier than with tulip iterators
			Iterator<node> *itNodes = graph->getNodes();

			while(itNodes->hasNext())
				nodes.push_back(itNodes->next());

			delete itNodes;
		}

		std::vector< node >::const_iterator itU, itV, it_end = nodes.end();
		for(itU = nodes.begin(); itU != it_end; ++itU) {
			u = *itU;
			cu = this->layout->getNodeValue(u);

			if(itU != it_end) {
				/*
				 * We will only tests nodes starting from the next one,
				 * as the ones before have already been tested
				 */
				itV = std::vector< node >::const_iterator(itU);
				++itV;
			}
			else break;

			for(; itV != it_end; ++itV) {
				v = *itV;
				cv = this->layout->getNodeValue(v);

				if(distance_type.getCurrentString().compare("Euclidian") == 0) {
					d = sqrt( (cu[0] - cv[0]) * (cu[0] - cv[0]) + (cu[1] - cv[1]) * (cu[1] - cv[1]) + (cu[2] - cv[2]) * (cu[2] - cv[2]) );
				} else if(distance_type.getCurrentString().compare("Manhattan") == 0) {
					d = fabs(cu[0] - cv[0]) + fabs(cu[1] - cv[1]) + fabs(cu[2] - cv[2]);
				} else if(distance_type.getCurrentString().compare("Chebychev") == 0) {
					d = max( fabs(cu[0] - cv[0]), max( fabs(cu[1] - cv[1]), fabs(cu[2] - cv[2]) ) );
				}

				if(d <= this->max_distance) {
					e = graph->addEdge(u, v);
					metric->setEdgeValue(e, d);
				}
			}
		}

		return true;
	}

	~EpsilonNeighborhoodConnectorOnLayout() {}
};

PLUGIN(EpsilonNeighborhoodConnectorOnLayout)
