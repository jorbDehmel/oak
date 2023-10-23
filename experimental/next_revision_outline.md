
# Next Major Revision Outline Planning Document

Jordan Dehmel

An outline of the upcoming changes in Oak. Do not erase anything
here, put new ideas at **top** (newest first).

----------------------------------------------------------------

## 8/21/23: Outline for revision 0.2.0 from 0.1.X

Goals:
- Improve compile-time efficiency
- Avoid recompilation whenever possible, especially .oak->.cpp
- Include only symbols which are from the current file in the
  output .cpp file if requested

Ideas:
- Add internal makefile in .oak_build/
- Use Acorn **only** to process .oak->.cpp (singular files)
- Possibly add another layer of abstraction, IE a new command
  line client to handle these things? (Name idea: sprig)

Outline:

  Save source file from symbol when added to symbol table. Each
  source file will entail a seperate output file. For each file,
  the most recent modification time will be recorded. If the
  .cpp file exists and has not been modified since then, do not
  recreate it. Otherwise, write all symbols obtained from it.

  As a consequence, in included files, only scrape the
  signatures of included functions. Structs and enums must be
  fully loaded, but regular code scopes should be ignored.

                               +- yes -> as usual
  Load -> is this the source? -|
                               |
                               +- no  -> does a .cpp exist?
                                                |
                                        +---------------+
                                        |               |
                                    up-to-date         no
                                        |               |
                                       \/              \/
                                    only sig          as usual

                               +- yes -> save all symbols
  Save -> is this the source? -|
                               +- no  -> does a .cpp exist?
                                                |
                                        +---------------+
                                        |               |
                                    up-to-date         no
                                        |               |
                                       \/              \/
                                    ignore            write file

  Save source file for all symbols in symbol table. Upon opening
  file, check for existing .cpp translation. If it has an up to
  date file, skip all non-generic code scopes. Continue as
  normal.

  At reconstruction time, construct map of all files and their
  corresponding symbols. Iterate over these files. For all files
  which are NOT up to date, rewrite their files.
