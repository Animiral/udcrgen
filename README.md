# udcrgen - Generate Unit Disk Contact Embeddings for Graphs

This program will create embeddings (drawings) of caterpillar and lobster graphs, if possible.

A separate generator program is included which systematically generates relevant graph instances.

## Usage of `udcrgen`

```
udcrgen [OPTIONS...] input.graph
```

The following options are available:

* `-a`, `--algorithm` `[knp|cleve]`
* `-i`, `--input-file` `<FILE>`
* `-o`, `--output-file` `<FILE>`
* `-j`, `--input-format` `<FORMAT>`
* `-f`, `--output-format` `<FORMAT>`
* `-e`, `--embed-order` `[lbs|bls|lsb|bsl|sbl|slb]`
 `-g`, `--gap` `<GAP>`
* `--`: end of options

For details on every command line item and synonymous options, please refer to the relevant following subsection.

### Supported Graph Classes

Caterpillars are trees which consist of a central chain (the “spine”) and any number of leaves connected to the spine.

Lobsters are trees which consist of a “spine”, any number of “branch” vertices connected to the spine and any number of leaves connected to the branches.

For details, consult the accompanying thesis paper.

### Algorithms

The program supports two classes of algorithms.

#### Klemz, Nöllenburg and Prutkin

This is the default.
It implements *proper* unit disk contact, i.e. two vertices are connected if and only if the disks in their output representation are in contact.
Use `-a knp`, `-a strict` or `-a strong` to choose this method.
*knp* stands for _Boris Klemz, Martin Nöllenburg, and Roman Prutkin_, authors of the paper _Recognizing weighted disk contact graphs_, which contains a description of this algorithm on caterpillars.

This program uses a configurable parameter called *gap* to visually distinguish disks which are not in contact.
Use `-g <gap>` to set the floating-point value. The default is `0.1`. The diameter of the unit disks before scaling is `1`.

#### Cleve

The alternative implements *weak* unit disk contact, i.e. if two vertices are connected, their two corresponding disks in the output are in contact.
Use `-a cleve` or `-a weak` to chose this method.
It is based on a description of the algorithm on caterpillars in the paper by _Jonas Cleve_: _Weak Unit Disk Contact Representations for
Graphs without Embedding_.

### Formats

The program currently has two input formats: `degrees` and `edgelist`.
Both are simple, whitespace-separated custom representations of graphs.

For `degrees`, the input file must contain a whitespace-separated list of the degrees of the spine vertices in order. The result will be a caterpillar.

For `edgelist`, the input file must contain one line for each edge, with the id of the from-vertex and the id of the to-vertex separated by a space.
ids are always integers.

The program supports SVG and IPE as output formats. Use `-f svg` and `-f ipe` respectively.

* SVG are Scalable Vector Graphics, a widespread XML-based format for vector graphics. Web browsers can display it.
* IPE is the native file format of [Ipe](https://ipe.otfried.org/), a drawing editor for creating figures in PDF format. 

Using `-f dump`, the program will output a text file generated from the internal result representation of the algorithm. It consists of lines of the form: `v -> p  (x, y)`, where

* _v_ is the number of an embedded vertex,
* _p_ is the number of the parent vertex to which _v_ connects and
* _(x, y)_ is the central coordinate of the embedded unit disk representing _v_.

### Heuristic Options

These options affect the heuristic used for embedding the graphs.

Use `-e lbs` to make the embedder place *l*eaf disks as soon as possible, *b*ranch disks as soon as possible and *s*pine disks as the last priority.
All other permutations are also valid. For example, with `-e sbl`, the complete spine will be placed before all the branches and finally all leaves.
Regardless of preference, no disk can ever be embedded before its parent disk.

## Usage of `gencases`

The program does not offer any options at the moment.
It populates the current directory with a predefined number of input files of the following categories, where `XX` is the number of spine vertices in the input:

* `maxbranchesXX.txt`: straight-spine lobsters with a lot of branches
* `maxleavesXX.txt`: straight-spine lobsters with a lot of leaves
* `onesided_bentXX.txt`: lobsters in with every other branch is very heavy, forcing bends
* `onesided_straightXX.txt`: lobsters in with every other branch is very heavy, but still allow for a straight spine

## Build Instructions

This project uses the CMake build system.

It only depends on the [GoogleTest library](https://github.com/google/googletest) for unit testing.

Run the following commands from the root of the repository directory to build.

```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```
