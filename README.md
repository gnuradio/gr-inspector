# gr-inspector
<center>
![gr-inspector logo](docs/doxygen/images/logo_body.png "The Inspector")
</center>
## Introduction
This GNU Radio module is part of the Google Summer of Code (GSoC) program 2016. The target is to develop a signal analysis / signal intelligence toolbox with the following capabilities:

- Automatic signal detection
- Automatic modulation classification
- GUI feedback
- Doxygen documentation

Additional functionality for the post-GSoC period can be:

- Signal allocation database
- Automatic demodulation
- Automatic decoding

Read the full proposal [here](https://github.com/sbmueller/gsoc-proposal/blob/master/sigint-proposal.pdf).

## Installation

To install the Inspector, the following dependencies are required:

- GNU Radio runtime
- gr-filter
- QT4
- QWT 6.1.0

Install by the following shell commands:

```
mkdir build
cd build
cmake .. -DENABLE_PYTHON=ON
make -j4
sudo make install
```

This module is still under development. Please let me know if you run into any issues on the master branch.

## Contact
Maintainer of this module:

Sebastian Müller<br/>
gsenpo[at]gmail.com

If you want to know more about the development process, please visit the [blog](https://grinspector.wordpress.com/).

If you have any questions or problems, the GNU Radio [mailing list](http://gnuradio.org/redmine/projects/gnuradio/wiki/MailingLists) is the right place to get help. There are many people who will provide assistance.
