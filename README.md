# Mfterm [![Build Status](https://travis-ci.org/4ZM/mfterm.svg?branch=master)](https://travis-ci.org/4ZM/mfterm)

mfterm is a terminal interface for working with Mifare Classic tags.

Installation
-----------
### Mac OS

`brew install mfterm`

### Linux

Precompiled package in [Kali Linux](!https://www.kali.org/)

Usage
-----------
Tab completion on commands is available. Also, commands that have file
name arguments provide tab completion on files. There is also a
command history, like in most normal shells.

Working with the mfterm program there are a few state variables that
are used.

Current Tag
-----------
The "current tag" is populated with the 'load' or 'read' commands. The
'read' command will read data from a physical tag and requires the
"current keys" to be set to the keys of the tag. Clear the "current
tag" by using the 'clear' command.

Display the "current tag" by using the 'print' command. The keys of
the "current tag" are displayed with the 'print keys' command. Note:
the tag keys could be different from the "current keys" displayed by
the 'keys print' command.

The data of the "current tag" can be manipulated with the 'set'
command.

The "current tag" can be persisted by writing it to a file with the
'save' command. It can also be written to a physical tag with the
'write' command. For the 'write' command to succeed, the "current
keys" have to be set to appropriate values. The 'write unlocked'
command can be used to write to block 0 on some 1k pirate cards.

If you are reading or loading a 1k tag, the mfterm program will still
use a full 4k tag to represent it. The last 3k will be all
zeroes. This is in analogy with the other libnfc tools.

Current Keys
------------
The "current keys" are used to authenticate when performing operations
on a physical tag. They can be displayed using the 'keys'
command. Clear the "current keys" by using the 'keys clear' command.

The keys are stored just like a tag in a file using the 'keys save',
but with all the data fields except the sector trailers cleared. The
keys can be loaded from a file, either a real tag dump or a key tag
dump, with the 'keys load' command.

The "current keys" can be set to match the "current tag" by using the
'keys import' command. It is also possible to manually set a key using
the 'keys set' command.

Use the 'keys test' command to test if the "current keys" can be used
to authenticate with a physical tag.

Dictionary
----------
A key dictionary can be imported from a file using the 'dict load'
command. This dictionary can then be used to perform a dictionary
attack on the sectors of a tag by using the 'dict attack' command.

The format of the dictionary file is simple. One key (6 bytes, 12 hex
characters) per line and # is a comment. 

Performing 'dict load' on several files will produce a dictionary that
is the union of those files. Duplicates will be removed.

To list all the keys in the dictionary, use the command 'dict'. To
clear the dictionary use 'dict clear'.

Other commands
--------------
Quit the mfterm program by issuing the 'quit' command.

Help is available by writing 'help'


MAC Computation
---------------

The function 'mac compute' is used for computing DES MACs (message
authentication codes). They require a 64 bit key that can be set using
the command 'mac key'. The same command, without arguments, is used to
display the current key.

The input to the DES MAC is UID + 14 left most bytes of the specified
block.

Using the command 'mac update' is shorthand for a MAC computation and
then setting the MAC of the same block.

Specification Files
-------------------
A specification file defines names for parts of the tag data. See the
file mfc-spec.txt for a sample specification.

Specification files are loaded with the command 'spec load'. They can
be cleared with 'spec clear'. To display the data structure loaded use
the command 'spec'.

Once a specification has been loaded, it can be used to access the
data in the tag by using a specification path. In the sample
specification, the path: '.sector_0.block_0.atqa', when entered in the
terminal, will display the two bytes of data starting with byte 6.


Building mfterm
---------------

Standard: ./configure; make; make install

See INSTALL file for details.


WARNING:
--------
The mfterm software is neither thoroughly tested nor widely used. It
likely contains a number of serious bugs that can be exploited to
compromise your computer. Do NOT run the mfterm software as a
privileged user (e.g. root), and ONLY load tag, dictionary and
specification files that you get from people you trust.
