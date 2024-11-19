# Restoration Issue Pocketknife

A tool to rip HE2 binary game resources from and to other formats, in particular JSON.

## How it works

`rip` can be used either as a standalone executable through the command line or as a library.
It is essentially the I/O core of DevTools isolated and cleaned up a bit.

`rip` heavily uses reflection to guide its serialization. This makes it easy to add new resources
to its set of supported resource types. Through this reflection approach it can generate both binary
and JSON versions of any file format it supports. It also supports HE2's builtin reflection system,
allowing it to convert any RFL data as well.

Currently the following resources are supported:

* Animation State Machine (.asm)
    * v103
* Object World / Gedit (.gedit)
    * v2
    * v3
* Vertex Animation Texture (.vat)
    * v1 - rangers variant
    * v1 - miller variant

## Usage

Simply run `rip.exe` with an input file as its argument. The conversion is dependent on the following options:

* Input format: What serialization format to convert from. `(binary, json)`
* Output format: What serialization format to convert to. `(binary, json)`
* Resource type: What kind of resource type to convert. `(asm, gedit, vat)`
* Version: Which version of the resource you want to convert. This depends on the selected resource. Valid choices:
  * ASM: `103`
  * Gedit: `2, 3`
  * VAT: `1-rangers, 1-miller`
* Game: Which game's RFL type system to use. This is important because not all games support all kinds of data,
  and some games default to different data types for the same data. Sonic Forces for example uses a different kind
  of array for arrays than Frontiers and SxSG. This option defaults to `miller`, which works for both Frontiers and SxSG. (`wars`, `rangers`, `miller`)
* Schema: Load RFL database info from an RFL Schema file. This is a file type that DevTools will be able to export soon.
* HedgeSet Template: Load RFL database info from a HedgeSet template. This does the same as the previous option, but loads
  reflection data from a HedgeSet template instead for compatibility.

`rip` will attempt to deduce plausible defaults for options that were not specified. If it cannot find a working set of options it
will return an error.

Because the binary formats contain pointers that are accessed directly, `rip` may simply crash if it tries to load a corrupt file.
The conversion was only successful if the message "Conversion successful." is printed.

### Full usage help output

```
rip.exe [OPTIONS] input [output]

  Restoration Issue Pocketknife

POSITIONALS:
  input TEXT:FILE REQUIRED    The input file.
  output TEXT                 The output file.

OPTIONS:
  -h,     --help              Print this help message and exit
  -g,     --game ENUM:value in {miller->2,rangers->1,wars->0} OR {2,1,0}
                              The target game.
  -r,     --resource-type ENUM:value in {asm->0,gedit->1,vat->3} OR {0,1,3}
                              The resource type.
  -v,     --version TEXT      The resource version. Available options are: asm -> 103; gedit ->
                              2, 3; vat -> 1-rangers, 1-miller
  -i,     --input-format ENUM:value in {binary->0,json->1} OR {0,1}
                              The input format.
  -o,     --output-format ENUM:value in {binary->0,json->1} OR {0,1}
                              The output format.
  -s,     --schema TEXT Excludes: --hedgeset-template
                              The RFL Schema file to use. (doesn't work yet)
  -t,     --hedgeset-template TEXT Excludes: --schema
                              The HedgeSet template file to use.
```
