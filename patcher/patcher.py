import struct

def patch_elf_needed(so_path, lib_to_add):
    with open(so_path, "rb") as f:
        data = bytearray(f.read())

    assert data[:4] == b'\x7fELF', "Не ELF файл!"
    fmt = "<" if data[5] == 1 else ">"

    lib_bytes = lib_to_add.encode("utf-8") + b'\x00'

    e_shoff     = struct.unpack_from(fmt + "Q", data, 40)[0]
    e_shentsize = struct.unpack_from(fmt + "H", data, 58)[0]
    e_shnum     = struct.unpack_from(fmt + "H", data, 60)[0]

    dynstr_offset  = None
    dynstr_size    = None
    dynamic_offset = None
    dynamic_size   = None

    for i in range(e_shnum):
        base    = e_shoff + i * e_shentsize
        sh_type = struct.unpack_from(fmt + "I", data, base + 4)[0]
        sh_off  = struct.unpack_from(fmt + "Q", data, base + 24)[0]
        sh_size = struct.unpack_from(fmt + "Q", data, base + 32)[0]

        if sh_type == 6:
            dynamic_offset = sh_off
            dynamic_size   = sh_size
        if sh_type == 3 and sh_off == 0x4f00:
            dynstr_offset = sh_off
            dynstr_size   = sh_size
            struct.pack_into(fmt + "Q", data, base + 32, sh_size + len(lib_bytes))
            print(f"sh_size обновлён: {sh_size} → {sh_size + len(lib_bytes)}")

    print(f".dynamic : offset={hex(dynamic_offset)} size={hex(dynamic_size)}")
    print(f".dynstr  : offset={hex(dynstr_offset)}  size={hex(dynstr_size)}")

    end_of_dynstr = dynstr_offset + dynstr_size
    padding_available = 0
    pos = end_of_dynstr
    while pos < len(data) and data[pos] == 0:
        padding_available += 1
        pos += 1

    print(f"Паддинг после dynstr: {padding_available} байт, нужно: {len(lib_bytes)}")

    if padding_available >= len(lib_bytes):
        str_index = dynstr_size
        for j, b in enumerate(lib_bytes):
            data[end_of_dynstr + j] = b
        print(f"Строка записана в паддинг: offset={hex(end_of_dynstr)}")
    else:
        print("ОШИБКА: недостаточно паддинга!")
        return

    entry_size  = 16
    num_entries = dynamic_size // entry_size

    for i in range(num_entries):
        base  = dynamic_offset + i * entry_size
        d_tag = struct.unpack_from(fmt + "q", data, base)[0]

        if d_tag == 10:  # DT_STRSZ
            new_size = dynstr_size + len(lib_bytes)
            struct.pack_into(fmt + "Q", data, base + 8, new_size)
            print(f"DT_STRSZ обновлён: {dynstr_size} → {new_size}")

        if d_tag == 0:  # DT_NULL
            struct.pack_into(fmt + "q", data, base,     1)
            struct.pack_into(fmt + "Q", data, base + 8, str_index)
            print(f"DT_NEEDED добавлен: '{lib_to_add}' str_index={str_index}")
            break

    out_path = so_path + ".patched"
    with open(out_path, "wb") as f:
        f.write(data)
    print(f"Сохранено: {out_path}")

if __name__ == "__main__":
    patch_elf_needed("libil2cpp.so", "libloader.so")