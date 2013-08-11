#ifndef DISTANCEFUNCTIONS_H
#define DISTANCEFUNCTIONS_H

#include <cmath>

template <typename TVector>
inline double euclidianDistance(const TVector &a, const TVector &b)
{
	double v = 0.0;
	size_t size = a.size();

	for(int i = 0; i < size; ++i) {
		v += (a[i] - b[i]) * (a[i] - b[i]);
	}

	return sqrt(v);
}

template <>
inline double euclidianDistance<tlp::Coord>(const tlp::Coord &a, const tlp::Coord &b)
{
	return sqrt( (a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]) );
}

template <typename TVector>
inline double manhattanDistance(const TVector &a, const TVector &b)
{
	double v = 0.0;
	size_t size = a.size();

	for(int i = 0; i < size; ++i) {
		v += fabs(a[i] - b[i]);
	}

	return v;
}

template <>
inline double manhattanDistance<tlp::Coord>(const tlp::Coord &a, const tlp::Coord &b)
{
	return fabs(a[0] - b[0]) + fabs(a[1] - b[1]) + fabs(a[2] - b[2]);
}

template <typename TVector>
inline double chebychevDistance(const TVector &a, const TVector &b)
{
	double v = 0.0;
	size_t size = a.size();

	for(int i = 0; i < size; ++i) {
		v = std::max(v, fabs(a[i] - b[i]));
	}

	return v;
}

template <>
inline double chebychevDistance<tlp::Coord>(const tlp::Coord &a, const tlp::Coord &b)
{
	return std::max(fabs(a[0] - b[0]), std::max(fabs(a[1] - b[1]), fabs(a[2] - b[2])));
}

#endif /* DISTANCEFUNCTIONS_H */
