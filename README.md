# udcrgen - Generate Unit Disk Contact Embeddings for Graphs

## Overview

This program will create embeddings (drawings) of caterpillar and lobster graphs, if possible.

Usage:

```
udcrgen [OPTIONS...] input.graph
```

The following options are available:

* `-a`, `--algorithm` `<ALGORITHM>`
* `-i`, `--input-file` `<FILE>`
* `-o`, `--output-file` `<FILE>`
* `-j`, `--input-format` `<FORMAT>`
* `-f`, `--output-format` `<FORMAT>`
* `-g`, `--gap` `<GAP>`
* `--`: end of options

## Supported Graph Classes

Caterpillars are trees which consist of a central chain (the “spine”) and any number of leaves connected to the spine.

Lobsters are trees which consist of a “spine”, any number of “branch” vertices connected to the spine and any number of leaves connected to the branches.

For details, consult the accompanying thesis paper.

## Algorithms

The program supports two classes of algorithms.

### Klemz, Nöllenburg and Prutkin

This is the default.
It implements *proper* unit disk contact, i.e. two vertices are connected if and only if the disks in their output representation are in contact.
Use `-a knp`, `-a strict` or `-a strong` to choose this method.
*knp* stands for _Boris Klemz, Martin Nöllenburg, and Roman Prutkin_, authors of the paper _Recognizing weighted disk contact graphs_, which contains a description of this algorithm on caterpillars.

This program uses a configurable parameter called *gap* to visually distinguish disks which are not in contact.
Use `-g <gap>` to set the floating-point value. The default is `0.1`. The diameter of the unit disks before scaling is `1`.

### Cleve

The alternative implements *weak* unit disk contact, i.e. if two vertices are connected, their two corresponding disks in the output are in contact.
Use `-a cleve` or `-a weak` to chose this method.
It is based on a description of the algorithm on caterpillars in the paper by _Jonas Cleve_: _Weak Unit Disk Contact Representations for
Graphs without Embedding_.

## Formats

The program currently has two input formats: `degrees` and `edgelist`.
Both are simple, whitespace-separated custom representations of graphs.

For `degrees`, the input file must contain a whitespace-separated list of the degrees of the spine vertices in order. The result will be a caterpillar.

For `edgelist`, the input file must contain one line for each edge, with the id of the from-vertex and the id of the to-vertex separated by a space.
ids are always integers.

The program supports SVG as an output format. Use `-f svg`.

Using `-f dump`, the program will output a text file generated from the internal result representation of the algorithm. It consists of lines of the form: `v -> p  (x, y)`, where

* _v_ is the number of an embedded vertex,
* _p_ is the number of the parent vertex to which _v_ connects and
* _(x, y)_ is the central coordinate of the embedded unit disk representing _v_.
