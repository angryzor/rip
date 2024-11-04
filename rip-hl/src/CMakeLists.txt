target_sources(rip-hl
    PRIVATE
        "rip/binary/containers/binary-file/BinaryFile.cpp"
    PUBLIC FILE_SET HEADERS FILES
        "rip/binary/stream.h"
        "rip/binary/serialization/BlobSerializer.h"
        "rip/binary/serialization/ReflectionSerializer.h"
        "rip/binary/types.h"
        "rip/reflection/traversals/simplerfl.h"
        "rip/util/memory.h"
        "rip/reflection/traversals/types.h"
        "rip/reflection/traversals/algorithm-base.h"
        "rip/reflection/opaque.h"
        "rip/util/magic.h"
        "rip/types.h"
        "rip/byteswap.h"
 "rip/binary/serialization/JsonSerializer.h")
