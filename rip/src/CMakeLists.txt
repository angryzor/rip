target_sources(rip
    PRIVATE
        "main.cpp"
        "config.cpp"
        "convert.cpp"
    PRIVATE FILE_SET HEADERS FILES
        "io/InputFile.h"
        "io/BinaryInputFile.h"
        "io/JsonInputFile.h"
        "io/load_input.h"
        "io/write_output.h"
        "io/load_hedgeset_template.h"
        "config.h"
        "convert.h"
        "util.h"
)
