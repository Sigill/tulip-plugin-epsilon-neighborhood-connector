# Tulip Epsilon-neighborhood plugin

## Description

This plugin builds epsilon-neighborhood on a set of nodes. It currently implements [enclidian](http://en.wikipedia.org/wiki/Euclidean_distance), [manhattan](http://en.wikipedia.org/wiki/Taxicab_geometry) and [chebychev](http://en.wikipedia.org/wiki/Taxicab_geometry) distance.

## Build

Launch one of the CMake project configuration tool and select your build directory. Set the CMAKE_MODULE_PATH variable to the location of the FindTULIP.cmake file (should be &lt;tulip_install_dir&gt;/share/tulip).

More informations [here](http://tulip.labri.fr/TulipDrupal/?q=node/1481).

## Use

* _Build epsilon-neighborhood on integerVector_
* _Build epsilon-neighborhood on doubleVector_
* _Build epsilon-neighborhood on layout_

All plugins use the following parameters:

* _metric_: The DoubleProperty where the distance will be stored.
* _distance type_: StringCollection indicating type of distance to use ("Euclidian", "Manhattan" or "Chebychev").
* _maximum distance_: double representing the maximum distance between two connected nodes.

In order to specify which property to use to compute the distances, the two plugins working with Vector properties use the _property_ parameter, while the one one working on the layout use _layout_ parameter.

## LICENSE

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).

