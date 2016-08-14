# The Inspector (gr-inspector)
<center>
![gr-inspector logo](docs/doxygen/images/logo_body.png "The Inspector")
</center>
## Introduction
This GNU Radio module is developed to realize signal analysis abilities in typical block-structure. The module is capable of the following:

- Energy detection of signals
- Filtering of detected signals
- OFDM parameter estimation (carrier spacing, symbol time)

## Installation

To install the Inspector, the following dependencies are required:

- GNU Radio core
- gr-filter
- QT4
- QWT 6.1.0

Install by the following shell commands:

```
mkdir build
cd build
cmake ..
make -j4
sudo make install
```

This module is still under development. Please let me know if you run into any issues on the master branch.

## Usage
Get inspired by the examples. There are flowgraphs for each block that should explain how to use them.

## Blocks

#### Signal Detector
Performs an energy detection on the input signal. A threshold in dB can be set or an autmatic threshold calculation can be used.

#### Inspector GUI
GUI where the spectrum is displayed along with signal markers for the detected signals. Also, signal can be selected manually in this block.

#### Signal Separator
Takes the signal info from the previous blocks and performs a down-mixing, filtering and decimation of each detected signal. The samples for each signal are passed on as a message.

#### Signal Extractor
Adapter block to extract the samples of one signal out of the messages from the Signal Separator. Output is again a complex stream.

#### OFDM Estimator
Estimates the parameters subcarrier spacing, symbol time, FFT size and cyclic prefix length for a input signal. The signal should not be oversampled for this block.

## Google Summer of Code
This GNU Radio module is part of the Google Summer of Code (GSoC) program 2016. The target is to develop a signal analysis / signal intelligence toolbox with the following capabilities:

- Automatic signal detection
- Automatic modulation classification
- OFDM parameter estimation and synchronization
- GUI feedback
- Doxygen documentation

Read the full proposal [here](https://github.com/sbmueller/gsoc-proposal/blob/master/sigint-proposal.pdf).

The modulation classification is developed by @chrisruk during ESA Summer of Code in Space program.

## Contact
Maintainer of this module:

Sebastian Müller<br/>
gsenpo[at]gmail.com

If you want to know more about the development process, please visit the [blog](https://grinspector.wordpress.com/).

If you have any questions or problems, the GNU Radio [mailing list](http://gnuradio.org/redmine/projects/gnuradio/wiki/MailingLists) is the right place to get help. There are many people who will provide assistance.
