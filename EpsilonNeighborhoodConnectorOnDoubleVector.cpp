#include <tulip/ImportModule.h>
#include <tulip/TulipPluginHeaders.h>
#include <tulip/StringCollection.h>
#include <stdexcept>
#include <vector>
#include <cmath>

#include "DistanceFunctions.h"

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
			HTML_HELP_DEF( "type", "DoubleVectorProperty" ) \
			HTML_HELP_BODY() \
			"The doubleVector property on which the distance will be computed." \
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

class EpsilonNeighborhoodConnectorOnDoubleVector:public tlp::Algorithm {
private:
	tlp::DoubleVectorProperty *property;
	tlp::DoubleProperty *metric;
	tlp::StringCollection distance_type;
	double (*distanceFunction)(const std::vector<double>&, const std::vector<double>&);
	double max_distance;

public:
	PLUGININFORMATIONS("Build epsilon-neighborhood on doubleVector", "Cyrille FAUCHEUX", "2013-08-08", "", "1.0", "Topology Update")

	EpsilonNeighborhoodConnectorOnDoubleVector(const tlp::PluginContext *context):Algorithm(context) {
		addInParameter< DoubleVectorProperty > ("property",         paramHelp[0], "data");
		addInParameter< DoubleProperty >       ("metric",           paramHelp[1], "viewMetric");
		addInParameter< StringCollection >     ("distance type",    paramHelp[2], SupportedDistances);
		addInParameter< double >               ("maximum distance", paramHelp[3], "1");
	}

	bool check(std::string &err) {
		try {
			if(dataSet == NULL)
				throw std::runtime_error("No dataset provided.");

			CHECK_PROP_PROVIDED("property",         this->property      );
			CHECK_PROP_PROVIDED("metric",           this->metric        );
			CHECK_PROP_PROVIDED("distance type",    this->distance_type );
			CHECK_PROP_PROVIDED("maximum distance", this->max_distance  );

			if(distance_type.getCurrentString().compare("Euclidian") == 0) {
				this->distanceFunction = euclidianDistance< std::vector< double > >;
			} else if(distance_type.getCurrentString().compare("Manhattan") == 0) {
				this->distanceFunction = manhattanDistance< std::vector< double > >;
			} else if(distance_type.getCurrentString().compare("Chebychev") == 0) {
				this->distanceFunction = chebychevDistance< std::vector< double > >;
			} else {
				throw std::runtime_error("Unknown distance type.");
			}

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
		std::vector< double > cu, cv;
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
			cu = this->property->getNodeValue(u);

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
				cv = this->property->getNodeValue(v);

				d = this->distanceFunction(cu, cv);

				if(d <= this->max_distance) {
					e = graph->addEdge(u, v);
					metric->setEdgeValue(e, d);
				}
			}
		}

		return true;
	}

	~EpsilonNeighborhoodConnectorOnDoubleVector() {}
};

PLUGIN(EpsilonNeighborhoodConnectorOnDoubleVector)
