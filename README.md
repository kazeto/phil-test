Phil-Test
====

# About
----
A software for unit test of [Phillip](https://github.com/kazeto/phillip).


# System Requirement
----
Phil-Test works under Unix based system, such as Linux, Mac.
Before using Phil-Test, the following softwares / libraries should be installed.

1. Softwares / libraries which Google Test requires
2. Softwares / libraries which Phillip requires
3. wget & unzip


# Usage
----
1. Get a distribution of Phil-Test from github.
   We recommend you to use git as following:

    $ git clone https://github.com/kazeto/phil-test.git

2. Get and compile Google Test:

    $ make configure

3. Get a distribution of Phillip from github and compile it as a library:

    $ git clone -b dev https://github.com/kazeto/phillip.git
    $ cd phillip
    $ python tools/configure.py
    $ make lib
    $ cd ..
    
4. Compile a binary of Phil-Test:

    $ make

5. Let's start!

    $ bin/phil-test

    