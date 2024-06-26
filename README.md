# ru_tts speech synthesizer

An alternative implementation of the Phonemophone-5 Russian speech
synthesizer by the international laboratory of intelligent systems
BelSInt (the Speech Recognition and Synthesis Lab of the Institute of
Technical Cybernetics of the Academy of Sciences of the Byelorussian
SSR). The source code of the original implementation has been lost.
This implementation is the result of a reverse engineering of
the SDRV resident speech driver for MS-DOS, and it is officially
approved for publication under a free license by Boris Lobanov, who is
the head of the laboratory and the author of the design solutions that
formed the basis of the speech synthesizer, and Alexander Ivanov,
who is an engineer of the laboratory and the developer of
the speech synthesizer's the original software implementation.


## Build instructions

To build the application you will need GNU
[Autoconf](https://www.gnu.org/software/autoconf/),
[Automake](https://www.gnu.org/software/automake/) and
[Libtool](https://www.gnu.org/software/libtool/) in addition to C
compiler and [Make](https://www.gnu.org/software/make/) utility. On a
Debian-like system all necessary tools can be installed by following command:

```bash
$ sudo apt install autoconf-archive automake libtool make
```

The building process itself consists of thre usual stages:

```bash
$ autoreconf -ifs
$ ./configure
$ make
```

The `configure` script produced on the first stage accepts several
options allowing us to control building process in some aspects. To
get a grasp run it as follows:

```bash
$ ./configure --help
```


## Cooperation with the RuLex pronunciation dictionary

This synthesizer can take advantage of the
[RuLex](https://github.com/poretsky/rulex) pronunciation dictionary.
By default the `rulex` library, that provides dictionary access means,
is not linked into the result executable as its mandatory part, but is
dynamically loaded by need at runtime if present.

Use configure option `--with-dictionary` if you want to link with the
`rulex` library on the application build stage. On the contrary, if no
*RuLex* cooperation is needed use configure option
`--without-dictionary`.


## Installation

After building the application can be installed as follows:

```bash
# make install
```

Besides that, there are ready to use packages for some releases of
*Ubuntu*, that can be directly installed as follows:

```bash
$ sudo add-apt-repository ppa:poretsky/a11y
$ sudo apt update
$ sudo apt install ru-tts
```


## Usage

See [README file](README) for usage details.


## Acknowledgements

Thanks to the following organizations and people for their contributions to the project:

* the international laboratory of intelligent systems BelSInt for the
  development of the original SDRV resident speech driver;
* Dmitry Paduchikh for help in the initial disassembly of the SDRV
  resident speech driver;
* Boris Lobanov for the creation of the Phonemophone-5 speech
  synthesizer and approval of the publication of the results of a
  reverse engineering of the SDRV resident speech driver under a free
  license.
* Alexander Ivanov for the development of the original software
  implementation of the Phonemophone-5 speech synthesizer and
  approval of the publication of the results of a reverse engineering
  of the SDRV resident speech driver under a free license.


## Scientific publications

In any papers or publications that use or mentioned the design solutions of the Phonemophone-5 speech synthesizer, please cite the following article:

* Ivanov A.N., Lobanov B.M. - PHONEMOPHONE speech synthesizer for CAD
  based on IBM PC // Abstracts report conf. "Theory and methods of
  creating intelligent CAD systems". - Minsk, 1992. - pp. 29-30 (in
  Russian).

The above article is a basic publication about the original
implementation of the Phonemophone-5 speech synthesizer for MS-DOS.

The microwave speech synthesis  method developed by Boris Lobanov and
used in the Phonemophone-5 is also described in the following
scientific publications:

* [Lobanov B.M. - Microwave Speech Synthesis from Text // Analysis and
  speech synthesis. - Minsk: the Institute of Technical Cybernetics of
  the ASB, 1991. - pp. 57-73 (in Russian).](publications/Lobanov_B.M._-_Microwave_Speech_Synthesis_from_Text_(in_Russian).pdf)
* Lobanov B.M. - Programming Model of Microwave Speech Synthesis from
  Text // Automatic Acoustic Pattern Recognition (AAPR-14). -
  Moscow, 1991. - pp. 82-84 (in Russian).
* Lobanov B.M. - Microwave Speech Synthesis // Automatic Acoustic
  Pattern Recognition (AAPR-16). - Moscow, 1991. - pp. 27-31 (in
  Russian).
* Lobanov B. - Microwave Speech Synthesis from Text // Proc. of the 24
  Fachkolloquim Informationstechnik. - Dresden, 1991. - pp. 118-120.

(For the original Russian titles of the scientific publications, see
the [Russian version of ReadMe](README_RU.md).)


## Related projects

There are several projects related to ru_tts speech synthesizer:

* [RuLex](https://github.com/poretsky/rulex) -
  Russian pronunciation dictionary for ru_tts
* [ru_tts for NVDA](https://github.com/kvark128/ru_tts-for-nvda) -
  add-on for NVDA screen reader with speech driver for ru_tts.
* [RU_TTS_EX](https://electrik-spb.ru/ru_tts/ru_tts_ex/) -
  extension to facilitate work with high-level languages
  and implement additional synthesizer capabilities
  (deprecated after ru_tts package release 6.0.4 and not supported)
