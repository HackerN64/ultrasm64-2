// This file will find all mips3 object files in an ar archive and set the ABI flags to O32
// this allows gcc to link them with the mips2 object files.
// Irix CC doesn't set the elf e_flags properly.
//
// In addition, it sorts symbol tables to put local symbols before global ones,
// which is required by modern ld.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARMAG  "!<arch>\n"
#define SARMAG 8

/* from elf.h */

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;

/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct
{
  unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  Elf32_Half    e_type;             /* Object file type */
  Elf32_Half    e_machine;          /* Architecture */
  Elf32_Word    e_version;          /* Object file version */
  Elf32_Addr    e_entry;            /* Entry point virtual address */
  Elf32_Off     e_phoff;            /* Program header table file offset */
  Elf32_Off     e_shoff;            /* Section header table file offset */
  Elf32_Word    e_flags;            /* Processor-specific flags */
  Elf32_Half    e_ehsize;           /* ELF header size in bytes */
  Elf32_Half    e_phentsize;        /* Program header table entry size */
  Elf32_Half    e_phnum;            /* Program header table entry count */
  Elf32_Half    e_shentsize;        /* Section header table entry size */
  Elf32_Half    e_shnum;            /* Section header table entry count */
  Elf32_Half    e_shstrndx;         /* Section header string table index */
} Elf32_Ehdr;

typedef struct {
    Elf32_Word  sh_name;
    Elf32_Word  sh_type;
    Elf32_Word  sh_flags;
    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;
    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addralign;
    Elf32_Word  sh_entsize;
} Elf32_Shdr;

typedef struct {
    Elf32_Word      st_name;
    Elf32_Addr      st_value;
    Elf32_Word      st_size;
    unsigned char   st_info;
    unsigned char   st_other;
    Elf32_Half      st_shndx;
} Elf32_Sym;

typedef struct {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
} Elf32_Rel;

typedef struct {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
    Elf32_Sword r_addend;
} Elf32_Rela;


/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define ELFMAG      "\177ELF"
#define SELFMAG     4

#define EI_CLASS    4       /* File class byte index */
#define ELFCLASS32  1       /* 32-bit objects */

#define EI_DATA     5       /* Data encoding byte index */
#define ELFDATA2MSB 2       /* 2's complement, big endian */

#define ELF32_ST_BIND(i)  ((i)>>4)
#define ELF32_R_SYM(i)    ((i)>>8)
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define SHT_SYMTAB  2
#define SHT_RELA    4
#define SHT_NOBITS  8
#define SHT_REL     9

#define STB_LOCAL   0

/* end from elf.h */

#define BSWAP16(x) ((uint16_t)(((x) & 0xff00) >> 8) | (uint16_t)(((x) & 0x00ff) << 8))
#define BSWAP32(x) (uint32_t)(BSWAP16((x) >> 16) | (uint32_t)(BSWAP16(x) << 16))

// the AR file is structured as followed
//"!<arch>" followed by 0x0A (linefeed) 8 characters
// then a file header that follows the following structure
// everything is represented using space padded characters
// the last two characters are alwos 0x60 0x0A
// then come the file contents
// you can find the location of the next header by adding file_size_in_bytes (after parsing)
// all file headers start at an even offset so if the file size in bytes is odd you have to add 1
// the first two "files" are special.  One is a symbol table with a pointer to the header of the file
// containing the symbol the other is an extended list of filenames
struct ar_header {
    char identifier[16];
    char file_modification_timestamp[12];
    char owner_id[6];
    char group_id[6];
    char file_mode[8];
    char file_size_in_bytes[10];
    char ending[2];
};

// These constants found by inspecting output of objdump
#define FLAGS_MIPS3 0x20
#define FLAGS_O32ABI 0x100000 

void *malloc_checked(size_t size)
{
    void *ret = calloc(size, 1);
    if (!ret) {
        printf("Failed to allocate memory\n");
        exit(-1);
    }
    return ret;
}

Elf32_Sym *syms;

int cmpsym(const void *lhs, const void *rhs)
{
    size_t ind1 = *(size_t *)lhs;
    size_t ind2 = *(size_t *)rhs;
    int local1 = ELF32_ST_BIND(syms[ind1].st_info) == STB_LOCAL;
    int local2 = ELF32_ST_BIND(syms[ind2].st_info) == STB_LOCAL;
    if (local1 != local2) {
        return local1 ? -1 : 1;
    }
    if (ind1 != ind2) {
        return ind1 < ind2 ? -1 : 1;
    }
    return 0;
}

int fix_mips_elf(FILE *f, size_t filesize)
{
    int changed = 0;
    uint8_t *buf = malloc_checked(filesize);
    if (1 != fread(buf, filesize, 1, f)) {
        printf("Failed to read ELF\n");
        return -1;
    }
    fseek(f, -(long)filesize, SEEK_CUR);

    if (filesize < sizeof(Elf32_Ehdr)) {
        printf("truncated header\n");
        return -1;
    }

    Elf32_Ehdr *hdr = (Elf32_Ehdr *)buf;
    if (hdr->e_ident[EI_CLASS] != ELFCLASS32 || hdr->e_ident[EI_DATA] != ELFDATA2MSB) {
        printf("Expected 32bit big endian object files\n");
        return -1;
    }

    if ((hdr->e_flags & 0xFF) == FLAGS_MIPS3 && (hdr->e_flags & FLAGS_O32ABI) == 0) {
        hdr->e_flags |= FLAGS_O32ABI;
        changed = 1;
    }

    size_t e_shoff = BSWAP32(hdr->e_shoff);
    size_t num_sections = BSWAP16(hdr->e_shnum);
    if (BSWAP16(hdr->e_shentsize) != sizeof(Elf32_Shdr) ||
            e_shoff > filesize ||
            filesize - e_shoff < num_sections * sizeof(Elf32_Shdr)) {
        printf("bad e_shentsize/e_shoff\n");
        return -1;
    }

    Elf32_Shdr *sections = (Elf32_Shdr *)(buf + e_shoff);
    Elf32_Shdr *symtab = NULL;
    for (size_t i = 0; i < num_sections; i++) {
        Elf32_Shdr *s = &sections[i];
        Elf32_Off sh_offset = BSWAP32(s->sh_offset);
        if (BSWAP32(s->sh_type) != SHT_NOBITS &&
                (sh_offset > filesize || filesize - sh_offset < BSWAP32(s->sh_size))) {
            printf("bad section data\n");
            return -1;
        }
        if (BSWAP32(sections[i].sh_type) == SHT_SYMTAB) {
            symtab = &sections[i];
        }
    }

    if (symtab == NULL) {
        printf("missing symtab\n");
        return -1;
    }

    Elf32_Word symtab_size = BSWAP32(symtab->sh_size);
    if (BSWAP32(symtab->sh_entsize) != sizeof(Elf32_Sym) ||
            symtab_size % sizeof(Elf32_Sym) != 0) {
        printf("bad symtab sh_size/sh_entsize\n");
        return -1;
    }

    syms = (Elf32_Sym *)(buf + BSWAP32(symtab->sh_offset));
    size_t num_syms = symtab_size / sizeof(Elf32_Sym);
    size_t num_locals = 0;
    size_t *new_sym_order = malloc_checked(num_syms * sizeof(*new_sym_order));
    for (size_t i = 0; i < num_syms; i++) {
        new_sym_order[i] = i;
        if (ELF32_ST_BIND(syms[i].st_info) == STB_LOCAL) {
            num_locals++;
        }
    }
    qsort(new_sym_order, num_syms, sizeof(*new_sym_order), cmpsym);

    Elf32_Sym *new_syms = malloc_checked(symtab_size);
    for (size_t i = 0; i < num_syms; i++) {
        new_syms[i] = syms[new_sym_order[i]];
    }
    if (memcmp(syms, new_syms, symtab_size)) {
        changed = 1;
    }
    memcpy(syms, new_syms, symtab_size);

    if (symtab->sh_info != BSWAP32(num_locals)) {
        changed = 1;
    }
    symtab->sh_info = BSWAP32(num_locals);

    size_t *new_sym_order_inv = malloc_checked(num_syms * sizeof(*new_sym_order));
    for (size_t i = 0; i < num_syms; i++) {
        new_sym_order_inv[new_sym_order[i]] = i;
    }

    for (size_t i = 0; i < num_sections; i++) {
        Elf32_Shdr *s = &sections[i];
        if (BSWAP32(s->sh_type) == SHT_REL) {
            if (BSWAP32(s->sh_entsize) != sizeof(Elf32_Rel) ||
                    BSWAP32(s->sh_size) % sizeof(Elf32_Rel) != 0) {
                printf("bad rel section\n");
                return -1;
            }
            Elf32_Rel *rels = (Elf32_Rel *)(buf + BSWAP32(s->sh_offset));
            size_t num_rels = BSWAP32(s->sh_size) / sizeof(Elf32_Rel);
            for (size_t j = 0; j < num_rels; j++) {
                Elf32_Word r_info = BSWAP32(rels[j].r_info);
                Elf32_Word sym = ELF32_R_SYM(r_info);
                Elf32_Word type = ELF32_R_TYPE(r_info);
                sym = (Elf32_Word)new_sym_order_inv[sym];
                r_info = ELF32_R_INFO(sym, type);
                rels[j].r_info = BSWAP32(r_info);
            }
        }

        if (BSWAP32(s->sh_type) == SHT_RELA) {
            if (BSWAP32(s->sh_entsize) != sizeof(Elf32_Rela) ||
                    BSWAP32(s->sh_size) % sizeof(Elf32_Rela) != 0) {
                printf("bad rela section\n");
                return -1;
            }
            Elf32_Rela *rels = (Elf32_Rela *)(buf + BSWAP32(s->sh_offset));
            size_t num_rels = BSWAP32(s->sh_size) / sizeof(Elf32_Rela);
            for (size_t j = 0; j < num_rels; j++) {
                Elf32_Word r_info = BSWAP32(rels[j].r_info);
                Elf32_Word sym = ELF32_R_SYM(r_info);
                Elf32_Word type = ELF32_R_TYPE(r_info);
                sym = (Elf32_Word)new_sym_order_inv[sym];
                r_info = ELF32_R_INFO(sym, type);
                rels[j].r_info = BSWAP32(r_info);
            }
        }
    }

    if (changed) {
        if (1 != fwrite(buf, filesize, 1, f)) {
            printf("Failed to write back ELF after patching.\n");
            return -1;
        }
        fseek(f, -(long)filesize, SEEK_CUR);
    }
    free(buf);
    free(new_syms);
    free(new_sym_order);
    free(new_sym_order_inv);
    return 0;
}

int fix_mips_ar(FILE *f)
{
    struct ar_header current_header;
    fseek(f, 0x8, SEEK_SET); // skip header, this is safe enough given that we check to make sure the
                             // file header is valid

    while (1 == fread(&current_header, sizeof(current_header), 1, f)) {
        if (current_header.ending[0] != 0x60 && current_header.ending[1] != 0x0A) {
            printf("Expected file header\n");
            return -1;
        }
        size_t filesize = atoi(current_header.file_size_in_bytes);

        if (filesize >= SELFMAG) {
            uint8_t magic[SELFMAG];

            if (1 != fread(magic, SELFMAG, 1, f)) {
                printf("Failed to read ELF magic\n");
                return -1;
            }
            fseek(f, -SELFMAG, SEEK_CUR);

            if (memcmp(magic, ELFMAG, SELFMAG) == 0) {
                if (fix_mips_elf(f, filesize)) {
                    return -1;
                }
            }
        }

        if (filesize % 2 == 1)
            filesize++;
        fseek(f, filesize, SEEK_CUR);
    }
    return 0;
}

int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "r+b");
    uint8_t magic[8];
    int status = 0;

    if (f == NULL) {
        printf("Failed to open file! %s\n", argv[1]);
        return -1;
    }
    if (1 != fread(&magic, sizeof(magic), 1, f)) {
        printf("Failed to read file magic\n");
        return -1;
    }
    rewind(f);
    if (!memcmp(ARMAG, magic, SARMAG)) {
        status = fix_mips_ar(f);
    } else if (!memcmp(ELFMAG, magic, SELFMAG)) {
        fseek(f, 0, SEEK_END);
        size_t filesize = ftell(f);
        rewind(f);
        status = fix_mips_elf(f, filesize);
    } else {
        printf("Unknown file magic: %02x%02x%02X%02X\n", magic[0], magic[1], magic[2], magic[3]);
        status = -1;
    }
    fclose(f);
    return status;
}
