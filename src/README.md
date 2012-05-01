# Congealing architecture

The congealing system is composed of several distinct layers:

1. Basic Objects

1. Source-Based Data Manipulation Library

1. File I/O 

1. Congealing Algorithm

Currently these all live in the `src/` directory, for convenient development, but at some point should be organized, and potentially deployed as separate packages.


## The Basic Objects Library

The following files are part of the Basic Objects


### libBaseTypes.h

definitions of various simple symantic types not inherent to C++

### Vector.h

a basic templated array class

### String.h, String.c++

a string class


### Stream.h

A memory based stream writing class. Essentially an expandable vector or string class.

### libSerialize.h

A set of serialization functions for basic types


### HashTable.h

associative array class

### Pixels.h

Simple pixel types and various support functions

### Promotion.h

Type promotion macros to ensure proper and efficient type conversion during mathematical operations.

### Points

A set of templated point classes for point manipulation

#### Point.h

Point base class

#### Point2D.h

Two dimensional points

#### Point3D.h

Three dimensional points

### libBarrier.c++, libBarrier.h

Workaround for system which lack the PTHREADS_BARRIER functionality

### libConfiguration.c++, libConfiguration.h

A configuration file reading library. Supports key-value stores and some rudamentary macro functionality.

### libRefCount.c++, libRefCount.h 

Reference counting memory mangement tools. Can operate in SAFE_MODE which is secure in SMP mode, on UNSAFE_MODE which has a small probability of corruption on large SMP systems, but yields substantial performance improvements.

### libUtility.h
 
Miscillaneous utility functions

### libDebug.h
 
A collection of debugging macros



## Source-Based Data Manipulation Library


### Source.h

The base object for the data manipulation object. It defines the interface as well
as set of macros to ensure optimal compilation of derived classes.

Sources are holders and providers of data at a collection of points. How a retrieves its data, and how that data is stored is abstracted away. 

Some sources, e.g. those derived from `SourceMemory` store their contents in memory. Other sources, those derived from `SourceAccessor` and `SourceTransform` instead look to other sources to provide the underlying data.

In this way, sources can be "stacked" upon one another to provide mutiple layers of access manipulation.

The source library leverages two properties of C++ which are typically at odds: templating and inheritence. Extensive use of compilation in-lining is used to achieve performance levels on par with custom coded data accessor objects.

Sources can be arbitrary dimension. This is typically the template parameter `DIMENSION`

Sources can have arbitrary "resolution" or "access precision", e.g. Integer, or Real.  This is typically the template parameter `PRECISION`

Sources can also refer to arbitrary data types, such as various classes of Pixels. This is typically the template parameter `DATA`


### SourceMemory.h

The base class for sources which look to a block of memory for their content.

#### Image.h

A SourceMemory derived source, with the addition of convenience functions.

#### PaddedImage.h

A derivative of `Image.h` which allows access beyond the bounds of the image, returning a constant padding value in such cases.

### SourceAccessor.h

The base classs for `Sources` which look to other sources for their underlying data.

In addition to the three template parameters of `DIMENSION`, `PRECISION` and `DATA` described above, they also take a `SOURCE` parameter which describes the type of the source to which the SourceAccessor looks.

Typically a source accessor will manipulate some or all of the coordinates (e.g. a warp), precision (e.g. a type conversion) or the data (e.g. contrast change) of the underlying data.


#### SourceAccessorAperature.h

A source which has the effect of changing the bounds of the underlying source. For example, if the underying source were a 200x200 2D image, which has dimensions which range from [0,200), [0,200), `SourceAccessorAperature` could be used to cut out a "window" at say [50,100), [50,100), or could be used to shift the entire space and make it accessible via [400,600), [1400,1600).

#### SourceAccessorConvolution.h

A read-only source which has the effect of convolving the data of the underlying source with the application of a filter which combines nearby data (e.g. blurring).


#### Interpolants

##### SourceAccessorNearestNeighbor.h
An accessor which converts from Integer `PRECISION` to Real `PRECISION` by returning the nearest point in the underlying integral data to the real coordiate specified.

##### SourceAccessorLinearInterpolate.h

An accessor which converts from Integer `PRECISION` to Real `PRECISION` using linear interpolation of the underlying data.


##### SourceAccessorCubicInterpolate.h

An accessor which converts from Integer `PRECISION` to Real `PRECISION` using cubic interpolation of the underlying data.

#### SourceAccessorEdgePad.h

Similar to PaddedImage, except it uses Accessor-style manipulation to achieve edge padding.


SourceAccessorEqualize.h
SourceAccessorRangeScale.h


SourceAccessorDistributionField.h
SourceAccessorPointwiseOperator.h
SourceAccessorSlice.h
SourceCombine.h
SourceCombineLayout.h
SourceCombinePointwiseOperator.h
SourceGenerics.h
SourceMaskCombine.h
SourceMaskCombinePointwiseOperator.h
SourceMaskCombineSelect.h
SourceMixed.h
SourceMixedPointwiseOperator.h
SourceRegistry.h
SourceTransform.h
SourceTransformContrast.h
SourceTransformDataScale.h
SourceTransformRemap.h
SourceTransformRotate.h
SourceTransformScale.h
SourceTransformSpatialScale.h
SourceTransformTranslate.h
SourceTransformWarp.h

libAssistants.h
libDownsample.h
libInterpolatedSource.h
libSourceOperations.h
Registry.h


1. File I/O 

libBrainWebIO.c++
libBrainWebIO.h
libMatlabIO.h
libNiftiIO.c++
libNiftiIO.h
libPGM.c++
libPGM.h


1. Optimization Algorithms

libOptimize_GreedySwarm.c++
libOptimize_GreedySwarm.h
libOptimize_NelderMead.c++
libOptimize_NelderMead.h
libOptimize_RandomWalk.c++
libOptimize_RandomWalk.h


1. Congealing Algorithm

CongealGroup.h
CongealGroupPhased.h
CongealRecipieOutput.h
Recipie.h
libNormalize.h


libErrorFunctions.h
libGaussian.h
main.c++


TestSuperresolution.c++
