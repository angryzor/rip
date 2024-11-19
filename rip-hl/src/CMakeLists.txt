target_sources(rip-hl
    PRIVATE
        "rip/binary/containers/binary-file/BinaryFile.cpp"
    PUBLIC FILE_SET HEADERS FILES
        "rip/util/byteswap.h"

        "rip/binary/stream.h"
        "rip/binary/serialization/BlobWorker.h"
        "rip/binary/serialization/BlobSerializer.h"
        "rip/binary/serialization/ReflectionSerializer.h"
        "rip/binary/serialization/JsonSerializer.h"
        "rip/binary/serialization/JsonDeserializer.h"
        
        "rip/schemas/hedgeset.h"
        "rip/util/memory.h"
)
