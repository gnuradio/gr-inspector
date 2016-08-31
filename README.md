<div align="center">

<img src="https://github.com/gnuradio/gr-inspector/blob/master/docs/doxygen/images/logo_body_big.png" />

</div>

# The Inspector (gr-inspector)

## Introduction
This GNU Radio module is developed to realize signal analysis abilities in typical block-structure. The module is capable of the following:
- Energy detection of continuous signals
- Filtering of detected signals
- OFDM parameter estimation (carrier spacing, symbol time)
- Blind OFDM synchronization
- Resampling of signals to constant rate
- 3D Visualisation of FAM data, from gr-specest
- Using TensorFlow models for AMC 

## Installation

To install the Inspector, the following dependencies are required:

- [GNU Radio](https://github.com/gnuradio/gnuradio)
- gr-analog, gr-blocks, gr-channel, gr-filter
- QT4
- QWT 6.1.0
- QwtPlot3D
- TensorFlow 0.10 ([pip installation](https://www.tensorflow.org/versions/r0.10/get_started/os_setup.html#pip-installation) - you need the version for Python 2.7)

Install by the following shell commands:

```
git clone git@github.com:gnuradio/gr-inspector.git
cd gr-inspector
mkdir build
cd build
cmake ..
make -j4
sudo make install
```

## Usage
Get inspired by the examples. There are flowgraphs for each block that should explain how to use them.

For detailed information about each block, visit the Doxygen HTML pages.
## Brief block description

A typical inspector flowgraph could look like this:
![A typical inspector flowgraph](https://grinspector.files.wordpress.com/2016/05/decentral.png)

#### Signal Detector
Performs an energy detection of continuous input signals. A threshold in dB can be set or an autmatic threshold calculation can be used.

#### Inspector GUI
GUI where the spectrum is displayed along with signal markers for the detected signals. Also, signal can be selected manually in this block.

#### Signal Separator
Takes the signal info from the previous blocks and performs a down-mixing, filtering and decimation of each detected signal. The samples for each signal are passed on as a message.

#### Signal Extractor
Adapter block to extract the samples of one signal out of the messages from the Signal Separator. Output is again a complex stream.

Resampling can be used to assure a constant output sample rate of the stream. (Needed for FM demod for instance.)

#### OFDM Estimator
Estimates the parameters subcarrier spacing, symbol time, FFT size and cyclic prefix length for a input signal. The signal should not be oversampled for this block.

#### OFDM Synchronizer
After OFDM parameter estimation, the signal is frequency synced and symbol beginnings are marked with stream tags.

#### 3D Data Visualisation
Visualisation of FAM data from gr-specest.

#### TensorFlow AMC Model
Loading of TensorFlow models for AMC.

## Google Summer of Code
This GNU Radio module is part of the Google Summer of Code (GSoC) program 2016. The target is to develop a signal analysis / signal intelligence toolbox with the following capabilities:

- Automatic signal detection
- Automatic modulation classification
- OFDM parameter estimation and synchronization
- GUI feedback
- Doxygen documentation

Read the full proposal [here](https://github.com/sbmueller/gsoc-proposal/blob/master/sigint-proposal.pdf).

This project was developed in cooperation with the [Communications Engineering Lab](http://www.cel.kit.edu/) of the Karlsruhe Institute of Technology.

The modulation classification is developed by @chrisruk during ESA Summer of Code in Space program.

## License
This software is Copyright © 2016 Sebastian Müller. It is free software, and is released under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html) License.

## Contact
Maintainer of this module:

Sebastian Müller<br/>
gsenpo[at]gmail.com

Christopher Richardson<br/>
chrisrichardsonuk[at]gmail.com

If you want to know more about the development process, please visit the [blog](https://grinspector.wordpress.com/).

If you have any questions or problems, the GNU Radio [mailing list](http://gnuradio.org/redmine/projects/gnuradio/wiki/MailingLists) is the right place to get help. There are many people who will provide assistance.
