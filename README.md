# Building the software

To build Congeal, use:

    make src -DCMAKE_BUILD_TYPE=Release
    make clean
    make

# Configuring

To configure, copy src/config to your working directory, and modify as desired. See the parameter documentation below.

# Running

Run the `congeal` executable with the syntax:

    congeal <config-file> [parameters]



# Congealing Parameter Documentation

## Table of contents

1. [congeal.initialsteps.rotation](#congeal.initialsteps.rotation)
1. [congeal.initialsteps.scale](#congeal.initialsteps.scale)
1. [congeal.initialsteps.translate](#congeal.initialsteps.translate)
1. [congeal.initialsteps.warp](#congeal.initialsteps.warp)
1. [congeal.inputfile.format](#congeal.inputfile.format)
1. [congeal.inputfile[]](#congeal.inputfile{})
1. [congeal.inputfiles](#congeal.inputfiles)
1. [congeal.inputfiles.list](#congeal.inputfiles.list)
1. [congeal.inputfiles.pattern](#congeal.inputfiles.pattern)
1. [congeal.optimize.algorithm](#congeal.optimize.algorithm)
1. [congeal.optimize.error](#congeal.optimize.error)
1. [congeal.optimize.progresspoints](#congeal.optimize.progresspoints)
1. [congeal.optimize[gradientdescent].kernel](#congeal.optimize{gradientdescent}.kernel)
1. [congeal.optimize[gradientdescent].walk](#congeal.optimize{gradientdescent}.walk)
1. [congeal.optimize[gradientdescent].walk.steps](#congeal.optimize{gradientdescent}.walk.steps)
1. [congeal.optimize[parzen].apriori](#congeal.optimize{parzen}.apriori)
1. [congeal.optimize[parzen].sigma](#congeal.optimize{parzen}.sigma)
1. [congeal.optimize[randomwalk].kernel](#congeal.optimize{randomwalk}.kernel)
1. [congeal.optimize[randomwalk].steps](#congeal.optimize{randomwalk}.steps)
1. [congeal.optimize[randomwalk].walk.directions](#congeal.optimize{randomwalk}.walk.directions)
1. [congeal.output.average.slices](#congeal.output.average.slices)
1. [congeal.output.average.slices.height](#congeal.output.average.slices.height)
1. [congeal.output.average.slices.postprocess](#congeal.output.average.slices.postprocess)
1. [congeal.output.average.slices.postprocess.dim#](#congeal.output.average.slices.postprocess.dim#)
1. [congeal.output.average.slices.width](#congeal.output.average.slices.width)
1. [congeal.output.average.volume](#congeal.output.average.volume)
1. [congeal.output.colors.mid](#congeal.output.colors.mid)
1. [congeal.output.colors.range](#congeal.output.colors.range)
1. [congeal.output.error](#congeal.output.error)
1. [congeal.output.matlab](#congeal.output.matlab)
1. [congeal.output.path](#congeal.output.path)
1. [congeal.output.prefix](#congeal.output.prefix)
1. [congeal.output.source.layout](#congeal.output.source.layout)
1. [congeal.output.source.layout.height](#congeal.output.source.layout.height)
1. [congeal.output.source.layout.postprocess](#congeal.output.source.layout.postprocess)
1. [congeal.output.source.layout.postprocess.dim#](#congeal.output.source.layout.postprocess.dim#)
1. [congeal.output.source.layout.width](#congeal.output.source.layout.width)
1. [congeal.output.source.slices](#congeal.output.source.slices)
1. [congeal.output.source.slices.height](#congeal.output.source.slices.height)
1. [congeal.output.source.slices.postprocess](#congeal.output.source.slices.postprocess)
1. [congeal.output.source.slices.postprocess.dim#](#congeal.output.source.slices.postprocess.dim#)
1. [congeal.output.source.slices.width](#congeal.output.source.slices.width)
1. [congeal.output.sourcegrid](#congeal.output.sourcegrid)
1. [congeal.schedule[&lt;schedule&gt;].cache](#congeal.schedule{&lt;schedule&gt;}.cache)
1. [congeal.schedule[&lt;schedule&gt;].downsample](#congeal.schedule{&lt;schedule&gt;}.downsample)
1. [congeal.schedule[&lt;schedule&gt;].optimize.affine](#congeal.schedule{&lt;schedule&gt;}.optimize.affine)
1. [congeal.schedule[&lt;schedule&gt;].optimize.iterations](#congeal.schedule{&lt;schedule&gt;}.optimize.iterations)
1. [congeal.schedule[&lt;schedule&gt;].optimize.samples](#congeal.schedule{&lt;schedule&gt;}.optimize.samples)
1. [congeal.schedule[&lt;schedule&gt;].optimize.warp[&lt;field&gt;]](#congeal.schedule{&lt;schedule&gt;}.optimize.warp{&lt;field&gt;})
1. [congeal.schedule[&lt;schedule&gt;].optimize.warp[&lt;field&gt;].size](#congeal.schedule{&lt;schedule&gt;}.optimize.warp{&lt;field&gt;}.size)
1. [congeal.schedules](#congeal.schedules)
1. [operation](#operation)

----------------------------
<a name='congeal.initialsteps.rotation' />
### **congeal.initialsteps.rotation**
#### Type: _real_

#### Suggested Range: 10-60

relative scaling of rotation parameters when computing kernels and step sizes. Scale: rotation in degrees


----------------------------
<a name='congeal.initialsteps.scale' />
### **congeal.initialsteps.scale**
#### Type: _real_

#### Suggested Range: 0.05-0.5

relative scaling of scaling parameters when computing kernels and step sizes. Scale: Scale as fraction of image size


----------------------------
<a name='congeal.initialsteps.translate' />
### **congeal.initialsteps.translate**
#### Type: _real_

#### Suggested Range: 0.01-0.5

relative scaling of translation parameters when computing kernels and step sizes. Scale: translation as fraction of image size


----------------------------
<a name='congeal.initialsteps.warp' />
### **congeal.initialsteps.warp**
#### Type: _real_

#### Suggested Range: 0.05-0.5

relative scaling of warp control point displacement when computing kernels and step sizes. Scale: Warp as fraction of control point's region
 Schedule options


----------------------------
<a name='congeal.inputfile.format' />
### **congeal.inputfile.format**
#### Type: _string_

format of input files. Currently only 'nifti' is supported


----------------------------
<a name='congeal.inputfile{}' />
### **congeal.inputfile[]**
#### Type: _string_

an explicit list of input files. Only the first
 congeal.inputfiles files are used. Can be used in conjunction
 with parameter substitution (e.g. {$n}) and incrementing
 (e.g. {++n})

#### Example:

     n -1
     congeal.inputfile[{++n}] ../input/testfiles/A.nii
     congeal.inputfile[{++n}] ../input/testfiles/B.nii
     congeal.inputfile[{++n}] ../input/testfiles/C.nii


----------------------------
<a name='congeal.inputfiles' />
### **congeal.inputfiles**
#### Type: _integer (1-300)_

number of input files to use. Use '0'
 for all files when used in conjuctions with congeal.inputfiles.list


----------------------------
<a name='congeal.inputfiles.list' />
### **congeal.inputfiles.list**
#### Type: _string_

a path to a file containing a list
 of input data files. The list file should
 contain one filename per line. Only congeal.inputfiles
 files will be used as input unless congeal.inputfiles is
 set to '0' in which case all the files in the list
 will be used


----------------------------
<a name='congeal.inputfiles.pattern' />
### **congeal.inputfiles.pattern**
#### Type: _string_

a printf()-style string taking one integer parameter
 to be replaced with the (zero-based) index of the file

#### Example:

     ../input/testfiles/%00d.nii


----------------------------
<a name='congeal.optimize.algorithm' />
### **congeal.optimize.algorithm**
#### Type: _enum (lbfgs,bruteforce,gradientdescent,randomwalk,neldermead)_

determines the optimization algorithm used
 lbfgs - a limited memory implementation of the
 Broyden-Fletcher-Goldfarb-Shanno method.
 NOTE: LBFGS is very slow, and does not seem to work all that well
 bruteforce - test a range of step sizes in each dimension. Take the best step.
 randomwalk - a beam-search type method
 gradientdescent - gradient descent optimization method

#### Example:

     congeal.optimize.algorithm lbfgs
     congeal.optimize.algorithm bruteforce
     congeal.optimize[bruteforce].range 20
     Gradient Descent parameters


----------------------------
<a name='congeal.optimize.error' />
### **congeal.optimize.error**
#### Type: _string {parzen, variance}_

selects the error metric to be used
 parzen -- entropy estimate based on Parzen density estimator
 variance -- variance of voxel stack


----------------------------
<a name='congeal.optimize.progresspoints' />
### **congeal.optimize.progresspoints**
#### Type: _integer_

#### Suggested Range: 1-100

determines how many output file sets will be
 generated during each schedule. Points are uniformly
 distributed every 1/progresspoints of the way. I.e.
 If progresspoints is 4, you'll get the 0%, 25%, 50%,
 and 75% and 100% points.
 initial step sizes for kernels


----------------------------
<a name='congeal.optimize{gradientdescent}.kernel' />
### **congeal.optimize[gradientdescent].kernel**
_only applies for congeal.optimize.algorithm gradientdescent_

#### Type: _real_

#### Suggested Range: 0.01 - 0.3

size of support to use for computing gradient. This factor is multipled
 by *.initialsteps to establish a radius for each dimension


----------------------------
<a name='congeal.optimize{gradientdescent}.walk' />
### **congeal.optimize[gradientdescent].walk**
_only applies for congeal.optimize.algorithm gradientdescent_

#### Type: _boolean_

if true, algorithm will take multiple steps in direction of gradient if beneficial


----------------------------
<a name='congeal.optimize{gradientdescent}.walk.steps' />
### **congeal.optimize[gradientdescent].walk.steps**
_only applies for congeal.optimize[gradientdescent].walk true_

#### Type: _integer (1-100)_

#### Suggested Range: 10-50

maximum number of steps to take in direction of gradient
 RandomWalk parameters


----------------------------
<a name='congeal.optimize{parzen}.apriori' />
### **congeal.optimize[parzen].apriori**
_only applies for congeal.optimize.error parzen_

#### Type: _real_

#### Suggested Range: 0.000000001 - 0.1

constant factor added to each Parzen estimate
 Output display options


----------------------------
<a name='congeal.optimize{parzen}.sigma' />
### **congeal.optimize[parzen].sigma**
_onlyu applies for congeal.optimize.error parzen_

#### Type: _real_

#### Suggested Range: 1%-20% of range of data

sigma of Gaussian used as kernel in Parzen density estimator. Measured in voxel intensity


----------------------------
<a name='congeal.optimize{randomwalk}.kernel' />
### **congeal.optimize[randomwalk].kernel**
_only applies for congeal.optimize.algorithm randomwalk_

#### Type: _real_

size of support to use for computing initial stepsize.
 This factor is multipled by *.initialsteps to establish
 a maximum step radius for each dimension


----------------------------
<a name='congeal.optimize{randomwalk}.steps' />
### **congeal.optimize[randomwalk].steps**
_only applies for congeal.optimize.algorithm randomwalk_

#### Type: _integer_

#### Suggested Range: 10-50

maximum number of steps to take along any beam


----------------------------
<a name='congeal.optimize{randomwalk}.walk.directions' />
### **congeal.optimize[randomwalk].walk.directions**
_only applies for congeal.optimize.algorithm randomwalk_

#### Type: _integer_

#### Suggested Range: 10-50

number of beams to try during each iteration
 Error function parameters


----------------------------
<a name='congeal.output.average.slices' />
### **congeal.output.average.slices**
#### Type: _bool_

determines whether or not the average slices should be generated


----------------------------
<a name='congeal.output.average.slices.height' />
### **congeal.output.average.slices.height**
#### Type: _integer_

#### Suggested Range: 256-2048

determines the height of the congealing average visualization


----------------------------
<a name='congeal.output.average.slices.postprocess' />
### **congeal.output.average.slices.postprocess**
#### Type: _format string (1 argument, the base name of the pgm image)_

a command to be executed for each dimension of the average slice file generated, unless
 a specific command is defined for that dimension. Use &quot;%1$s&quot; to get the file base name

#### Example:

     congeal.output.average.slices.postprocess convert %1$s.pgm %1$s.png


----------------------------
<a name='congeal.output.average.slices.postprocess.dim#' />
### **congeal.output.average.slices.postprocess.dim#**
#### Type: _format string (1 argument, the base name of the pgm image)_

a command to be executed for the specified dimension (0,1, or 2) of
 each average slice file generated. Use &quot;%1$s&quot; to get the file base name

#### Example:

     congeal.output.average.slices.postprocess.dim0 convert %1$s.pgm -flip %1$s.png


----------------------------
<a name='congeal.output.average.slices.width' />
### **congeal.output.average.slices.width**
#### Type: _integer_

#### Suggested Range: 256-2048

determines the width of the congealing average visualization


----------------------------
<a name='congeal.output.average.volume' />
### **congeal.output.average.volume**
#### Type: _bool_

determines whether or not the average volume should be generated


----------------------------
<a name='congeal.output.colors.mid' />
### **congeal.output.colors.mid**
#### Type: _integer_

#### Suggested Range: 25% of data range (e.g. 64 for UCHAR)

color equalization intercept. This value
 determines which data value will be mapped to mid gray

#### See Also: DATARANGE compilation flag


----------------------------
<a name='congeal.output.colors.range' />
### **congeal.output.colors.range**
#### Type: _integer (0-4096)_

#### Suggested Range: 25% of data range (e.g. 64 for UCHAR)

color equalization slope.
 This value determines the relationship between changes
 in data value and changes in output image gray value

#### See Also: DATARANGE_0_TO_255 compilation flag


----------------------------
<a name='congeal.output.error' />
### **congeal.output.error**
#### Type: _bool_

determines whether or not full parzen error files should be computed


----------------------------
<a name='congeal.output.matlab' />
### **congeal.output.matlab**
#### Type: _bool_

#### Suggested Range: 250-10000

determines whether or not matlab transform files should be generated


----------------------------
<a name='congeal.output.path' />
### **congeal.output.path**
#### Type: _string_

directory into which outputfiles are saved. This value can
 include an absolute or relative path. Should end in a slash

#### Example:

     congeal.output.path /tmp/output/congeal/


----------------------------
<a name='congeal.output.prefix' />
### **congeal.output.prefix**
#### Type: _string_

string prepended to the filenames of the outputfiles

#### Example:

     congeal.output.prefix run12-


----------------------------
<a name='congeal.output.source.layout' />
### **congeal.output.source.layout**
#### Type: _bool_

determines whether or not the source layout montage should be generated


----------------------------
<a name='congeal.output.source.layout.height' />
### **congeal.output.source.layout.height**
#### Type: _integer_

#### Suggested Range: 256-2048

determines the width of the source layout montage


----------------------------
<a name='congeal.output.source.layout.postprocess' />
### **congeal.output.source.layout.postprocess**
#### Type: _format string (1 argument, the base name of the pgm image)_

a command to be executed for each dimension of each layout file generated, unless a specific command is defined for that dimension. Use &quot;%1$s&quot; to get the file base name

#### Example:

     congeal.output.source.layout.postprocess convert %1$s.pgm %1$s.png


----------------------------
<a name='congeal.output.source.layout.postprocess.dim#' />
### **congeal.output.source.layout.postprocess.dim#**
#### Type: _format string (1 argument, the base name of the pgm image)_

a command to be executed for the specified dimension (0,1, or 2) of
 each layout file generated. Use &quot;%1$s&quot; to get the file base name

#### Example:

     congeal.output.source.layout.postprocess.dim0 convert %1$s.pgm -flip %1$s.png


----------------------------
<a name='congeal.output.source.layout.width' />
### **congeal.output.source.layout.width**
#### Type: _integer_

#### Suggested Range: 256-2048

determines the width of the source layout montage


----------------------------
<a name='congeal.output.source.slices' />
### **congeal.output.source.slices**
#### Type: _bool_

determines whether or not the individual source slices should be generated


----------------------------
<a name='congeal.output.source.slices.height' />
### **congeal.output.source.slices.height**
#### Type: _integer_

#### Suggested Range: 256-2048

determines the height of the source slices images


----------------------------
<a name='congeal.output.source.slices.postprocess' />
### **congeal.output.source.slices.postprocess**
#### Type: _format string (1 argument, the base name of the pgm image)_

a command to be executed for each dimension of each source slice file generated, unless a specific command is defined for that dimension. Use &quot;%1$s&quot; to get the file base name

#### Example:

     congeal.output.source.slices.postprocess convert %1$s.pgm %1$s.png


----------------------------
<a name='congeal.output.source.slices.postprocess.dim#' />
### **congeal.output.source.slices.postprocess.dim#**
#### Type: _format string (1 argument, the base name of the pgm image)_

a command to be executed for the specified dimension (0,1, or 2) of
 each source slice file generated. Use &quot;%1$s&quot; to get the file base name

#### Example:

     congeal.output.source.slices.postprocess.dim0 convert %1$s.pgm -flip %1$s.png


----------------------------
<a name='congeal.output.source.slices.width' />
### **congeal.output.source.slices.width**
#### Type: _integer_

#### Suggested Range: 256-2048

determines the width of the source slices images


----------------------------
<a name='congeal.output.sourcegrid' />
### **congeal.output.sourcegrid**
#### Type: _integer_

#### Suggested Range: 1-49

determines how many of the transformed source values are showm in the *-inputs* images


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.cache' />
### **congeal.schedule[&lt;schedule&gt;].cache**
_the value of &lt;schedule&gt; determines which congealing schedule this parameter affects_

#### Type: _boolean_

determines whether or not this schedules results can be retried from a previous run


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.downsample' />
### **congeal.schedule[&lt;schedule&gt;].downsample**
_the value of &lt;schedule&gt; determines which congealing schedule this parameter affects_

#### Type: _integer_

determines how many times the input data should be downsampled (by factors of 2 in each dimension) prior to congealing


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.optimize.affine' />
### **congeal.schedule[&lt;schedule&gt;].optimize.affine**
_the value of &lt;schedule&gt; determines which congealing schedule this parameter affects_

#### Type: _boolean_

determines if affine parameters should be optimized or left fixed


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.optimize.iterations' />
### **congeal.schedule[&lt;schedule&gt;].optimize.iterations**
_the value of &lt;schedule&gt; determines which congealing schedule this parameter affects_

#### Type: _integer_

number of optimzation iterations to be taken in this schedule


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.optimize.samples' />
### **congeal.schedule[&lt;schedule&gt;].optimize.samples**
_the value of &lt;schedule&gt; determines which congealing schedule this parameter affects_

#### Type: _integer_

number of samples to be compared in each transformed input volume


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.optimize.warp{&lt;field&gt;}' />
### **congeal.schedule[&lt;schedule&gt;].optimize.warp[&lt;field&gt;]**
_&lt;field&gt; can be 0,1, or 2_

#### Type: _boolean_

determines if the B-spline parameters for B-spline field &lt;field&gt; should be optimized or left fixed


----------------------------
<a name='congeal.schedule{&lt;schedule&gt;}.optimize.warp{&lt;field&gt;}.size' />
### **congeal.schedule[&lt;schedule&gt;].optimize.warp[&lt;field&gt;].size**
_&lt;field&gt; can be 0,1, or 2_

#### Type: _integer_

determines number of support points in each dimension of of B-Spline mesh


----------------------------
<a name='congeal.schedules' />
### **congeal.schedules**
#### Type: _integer (1-10)_

establishes how many schedules will be run

#### Example:

     congeal.schedules 7


----------------------------
<a name='operation' />
### **operation**
#### Type: _string_

the test to be performed. For congealing, use &quot;congeal&quot;

