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
			HTML_HELP_DEF( "type", "Property" ) \
			HTML_HELP_BODY() \
			"The property on which the distance will be computed." \
			HTML_HELP_CLOSE(),

		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "DoubleProperty" ) \
			HTML_HELP_BODY() \
			"The property in which to store the computed distance." \
			HTML_HELP_CLOSE(),

		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "String" ) \
			HTML_HELP_DEF( "Values", "Euclidian<br>Manhattan<br>Chebychev" )
			HTML_HELP_BODY() \
			"The type of distance to use. Not required when using an IntegerProperty or a DoubleProperty." \
			HTML_HELP_CLOSE(),

		HTML_HELP_OPEN() \
			HTML_HELP_DEF( "type", "double" ) \
			HTML_HELP_BODY() \
			"Distance above which two nodes will not be connected." \
			HTML_HELP_CLOSE()
	};
}

class EpsilonNeighborhoodConnector:public tlp::Algorithm {
private:
	tlp::PropertyInterface *features_property;
	tlp::DoubleProperty *metric_property;
	tlp::StringCollection distance_type;
	double max_distance;

	enum metric_t { EUCLIDIAN = 0, MANHATTAN, CHEBYCHEV };
	metric_t metric;

	enum property_t { NUMERIC, INTEGERVECTOR, DOUBLEVECTOR, LAYOUT };
	property_t property_type;

public:
	PLUGININFORMATIONS("Build epsilon-neighborhood", "Cyrille FAUCHEUX", "2013-08-10", "", "1.0", "Topology Update")

	EpsilonNeighborhoodConnector(const tlp::PluginContext *context):Algorithm(context) {
		addInParameter< PropertyInterface* > ("property",         paramHelp[0], "data");
		addInParameter< DoubleProperty >     ("metric",           paramHelp[1], "viewMetric");
		addInParameter< StringCollection >   ("distance type",    paramHelp[2], "Euclidian;Manhattan;Chebychev");
		addInParameter< double >             ("maximum distance", paramHelp[3], "1");
	}

	bool check(std::string &err) {
		try {
			if(dataSet == NULL)
				throw std::runtime_error("No dataset provided.");

			CHECK_PROP_PROVIDED("property",         this->features_property );
			CHECK_PROP_PROVIDED("metric",           this->metric_property );
			CHECK_PROP_PROVIDED("maximum distance", this->max_distance  );

			if (dynamic_cast< NumericProperty* >(this->features_property)) {
				this->property_type = NUMERIC;
			} else if (dynamic_cast< IntegerVectorProperty* >(this->features_property)) {
				this->property_type = INTEGERVECTOR;
			} else if (dynamic_cast< DoubleVectorProperty* >(this->features_property)) {
				this->property_type = DOUBLEVECTOR;
			} else if (dynamic_cast< LayoutProperty* >(this->features_property)) {
				this->property_type = LAYOUT;
			} else {
				throw std::runtime_error("\"property\" must be a property of one of the following types: IntegerProperty, DoubleProperty, IntegerVectorProperty, DoubleVectorProperty, LayoutProperty.");
			}

			if(NUMERIC != this->property_type)
				CHECK_PROP_PROVIDED("distance type", this->distance_type );

			if(distance_type.getCurrentString().compare("Euclidian") == 0) {
				this->metric = EUCLIDIAN;
			} else if(distance_type.getCurrentString().compare("Manhattan") == 0) {
				this->metric = MANHATTAN;
			} else if(distance_type.getCurrentString().compare("Chebychev") == 0) {
				this->metric = CHEBYCHEV;
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
		std::vector< node > nodes;
		nodes.reserve(graph->numberOfNodes());

		{ // Building a list of the nodes, it will make iterating through it easier than with tulip iterators
			Iterator<node> *itNodes = graph->getNodes();

			while(itNodes->hasNext())
				nodes.push_back(itNodes->next());

			delete itNodes;
		}

		tlp::NumericProperty*       np;
		tlp::IntegerVectorProperty* ivp;
		tlp::DoubleVectorProperty*  dvp;
		tlp::LayoutProperty*        lp;

		switch(this->property_type) {
			case NUMERIC:
				np = dynamic_cast< NumericProperty* >(this->features_property);
				break;
			case INTEGERVECTOR:
				ivp = dynamic_cast< IntegerVectorProperty* >(this->features_property);
				break;
			case DOUBLEVECTOR:
				dvp = dynamic_cast< DoubleVectorProperty* >(this->features_property);
				break;
			case LAYOUT:
				lp = dynamic_cast< LayoutProperty* >(this->features_property);
				break;
		}

		node u, v;
		edge e;
		double d;
		int step = 0, max_step = graph->numberOfNodes();
		std::vector< node >::const_iterator itU, itV, it_end = nodes.end();

		for(itU = nodes.begin(); itU != it_end; ++itU) {
			u = *itU;

			if(itU != it_end) {
				/*
				 * We will only tests nodes starting from the next one,
				 * as the ones before have already been tested
				 */
				itV = std::vector< node >::const_iterator(itU);
				++itV;
			}
			else break;


			if(NUMERIC == this->property_type) {
				double cu = np->getNodeDoubleValue(u);

				for(; itV != it_end; ++itV) {
					v = *itV;

					double d = fabs(cu - np->getNodeDoubleValue(v));
					if(d <= this->max_distance) {
						e = graph->addEdge(u, v);
						this->metric_property->setEdgeValue(e, d);
					}
				}
			} else if(INTEGERVECTOR == this->property_type) {
				std::vector< int > cu = ivp->getNodeValue(u);

				for(; itV != it_end; ++itV) {
					v = *itV;
					std::vector< int > cv = ivp->getNodeValue(v);

					if(EUCLIDIAN == this->metric)
						d = euclidianDistance< std::vector< int > >(cu, cv);
					else if(MANHATTAN == this->metric)
						d = manhattanDistance< std::vector< int > >(cu, cv);
					else if(CHEBYCHEV == this->metric)
						d = chebychevDistance< std::vector< int > >(cu, cv);

					if(d <= this->max_distance) {
						e = graph->addEdge(u, v);
						this->metric_property->setEdgeValue(e, d);
					}
				}
			} else if(DOUBLEVECTOR == this->property_type) {
				std::vector< double > cu = dvp->getNodeValue(u);

				for(; itV != it_end; ++itV) {
					v = *itV;
					std::vector< double > cv = dvp->getNodeValue(v);

					if(EUCLIDIAN == this->metric)
						d = euclidianDistance< std::vector< double > >(cu, cv);
					else if(MANHATTAN == this->metric)
						d = manhattanDistance< std::vector< double > >(cu, cv);
					else if(CHEBYCHEV == this->metric)
						d = chebychevDistance< std::vector< double > >(cu, cv);

					if(d <= this->max_distance) {
						e = graph->addEdge(u, v);
						this->metric_property->setEdgeValue(e, d);
					}
				}
			} else if(LAYOUT == this->property_type) {
				tlp::Coord cu = lp->getNodeValue(u);

				for(; itV != it_end; ++itV) {
					v = *itV;
					tlp::Coord cv = lp->getNodeValue(v);

					if(EUCLIDIAN == this->metric)
						d = euclidianDistance< tlp::Coord >(cu, cv);
					else if(MANHATTAN == this->metric)
						d = manhattanDistance< tlp::Coord >(cu, cv);
					else if(CHEBYCHEV == this->metric)
						d = chebychevDistance< tlp::Coord >(cu, cv);

					if(d <= this->max_distance) {
						e = graph->addEdge(u, v);
						this->metric_property->setEdgeValue(e, d);
					}
				}
			}

			if (pluginProgress && ((++step % 10) == 0)) {
				ProgressState state = pluginProgress->progress(++step, max_step);

				if (state != TLP_CONTINUE)
					return state != TLP_CANCEL;
			}
		}

		return true;
	}

	~EpsilonNeighborhoodConnector() {}
};

PLUGIN(EpsilonNeighborhoodConnector)
