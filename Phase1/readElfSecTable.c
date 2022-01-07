#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "readElfSecTable.h"
#include "readElfHeader.h"

/* Fichier principal de l'etape 2 : affichage de la table des sections */

/*	find_type(long num, char *sh_type)
		Lit la valeur donnee dans le premier parametre afin de definir la valeur du second
*/
void find_type(long num, char *sh_type)
{
	// Documentation chapitre 1-11 Figure 1-9
	switch (num)
	{
	case 0:
		strcpy(sh_type, "NULL");
		break;
	case 1:
		strcpy(sh_type, "PROGBITS");
		break;
	case 2:
		strcpy(sh_type, "SYMTAB");
		break;
	case 3:
		strcpy(sh_type, "STRTAB");
		break;
	case 4:
		strcpy(sh_type, "RELA");
		break;
	case 5:
		strcpy(sh_type, "HASH");
		break;
	case 6:
		strcpy(sh_type, "DYNAMIC");
		break;
	case 7:
		strcpy(sh_type, "NOTE");
		break;
	case 8:
		strcpy(sh_type, "NOBITS");
		break;
	case 9:
		strcpy(sh_type, "REL");
		break;
	case 10:
		strcpy(sh_type, "SHLIB");
		break;
	case 11:
		strcpy(sh_type, "DYNSYM");
		break;
	case 0xe:
		strcpy(sh_type, "INIT_ARRAY");
		break;
	case 0xf:
		strcpy(sh_type, "FINI_ARRAY");
		break;
	case 0x6ffffff6:
		strcpy(sh_type, "GNU_HASH");
		break;
	case 0x6ffffffd:
		strcpy(sh_type, "VERDEF");
		break;
	case 0x6ffffffe:
		strcpy(sh_type, "VERNEED");
		break;
	case 0x6fffffff:
		strcpy(sh_type, "VERSYM");
		break;
	case 0x70000000:
		strcpy(sh_type, "LOPROC");
		break;
	case 0x7fffffff:
		strcpy(sh_type, "HIPROC");
		break;
	case 0x80000000:
		strcpy(sh_type, "LOUSER");
		break;
	case 0xffffffff:
		strcpy(sh_type, "HIUSER");
		break;
	default:
		strcpy(sh_type, "UNKOWN");
		break;
	}
}

/*	revstr(char *str)
		Prend une chaine de caracteres et modifie sa valeur en l'inversant
		utilisee pour faire la transition big Endian / little Endian
*/
void revstr(char *str)
{
	int i, len, temp;
	len = strlen(str);
	for (i = 0; i < len / 2; i++)
	{
		temp = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = temp;
	}
}

/*	find_flags(char *tab, int n)
		Traduction des actions effectuees en flags pour une section
*/
void find_flags(char *tab, int n)
{
	int i, j = 0;
	strcpy(tab, "");
	// les valeurs de tabVal proviennent du site https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/6n33n7fcj/index.html
	int tabVal[11] = {0x400,0x200,0x100,0x80,0x40, 0x20, 0x10, 0x4, 0x2, 0x1, 0x0};
	char tabChar[11] = {'T','G','O','L','I', 'S', 'M', 'X', 'A', 'W', ' '};
	for (i = 0; n > 0x0; i++)
	{
		if (n >= tabVal[i])
		{
			tab[j] = tabChar[i];
			n -= tabVal[i];
			j++;
		}
	}
	tab[j] = '\0';
}

/*	lecture_section_table(FILE *elfFile, Elf64_Ehdr header)
		Lit la table
*/
liste_sections lecture_section_table(FILE *elfFile, Elf32_Ehdr header){
	liste_sections liste;
	char flags[15] = "";
	Elf32_Shdr sectHdr;
	char *sectNames = NULL;
	char sh_type[25] = "";


	// Placement du curseur au debut de la premiere section
	fseek(elfFile, header.e_shoff + header.e_shstrndx * header.e_shentsize, SEEK_SET);

	// Lecture de l'en-tete de la premiere section
	fread(&sectHdr, 1, sizeof(sectHdr), elfFile);

	// Allocation dans sectNames du nom de la premiere section
	sectNames = malloc(sectHdr.sh_size);
	fseek(elfFile, sectHdr.sh_offset, SEEK_SET);
	fread(sectNames, 1, sectHdr.sh_size, elfFile);

	// Parcours de toutes les sections
	
	liste.nb_sections = header.e_shnum;
	liste.tab = malloc(sizeof(section) * header.e_shnum);

	for (int i = 0; i < header.e_shnum; i++)
	{
		char *sh_name = "";
		// Lecture de l'en-tete de la section courante
		fseek(elfFile, header.e_shoff + i * sizeof(liste.tab[i].sec), SEEK_SET);
		fread(&liste.tab[i].sec, 1, sizeof(liste.tab[i].sec), elfFile);

		// Allocation memoire des donnees de la taille de la section definie par tab[i]
		liste.tab[i].tabDonnee = malloc(liste.tab[i].sec.sh_size);

		// Lecture des flags la section courante
		find_flags(flags, liste.tab[i].sec.sh_flags);
		revstr(flags);
		
		strcpy(liste.tab[i].flag, flags);

		// Lecture du type de la section courante
		find_type(liste.tab[i].sec.sh_type, sh_type);
		sh_name = sectNames + liste.tab[i].sec.sh_name;
		if(strlen(sh_name) > 16){
			sh_name[17] = '\0';
		}
		strcpy(liste.tab[i].name, sh_name);
		strcpy(liste.tab[i].type, sh_type);
	}


	free(sectNames);
	return liste;
}

/*	affiche_section_table(FILE *elfFile, Elf64_Ehdr header)
		Affichage de la table des sections et des informations pour chaque section
*/
void affiche_section_table(liste_sections liste, uint32_t offset){

	printf("There are %d section headers, starting at offset 0x%x:\n\n",
		   liste.nb_sections, offset);
	printf("Section Headers:\n  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n");

	for (int i = 0; i < liste.nb_sections; i++){

		printf("  [%2d] %-17s %-15s %08x %06x ", i, liste.tab[i].name, liste.tab[i].type,
			   liste.tab[i].sec.sh_addr, liste.tab[i].sec.sh_offset);
		printf("%06x %02x %3s %2d  %2d %2d\n", liste.tab[i].sec.sh_size,
			   liste.tab[i].sec.sh_entsize, liste.tab[i].flag, liste.tab[i].sec.sh_link,
			   liste.tab[i].sec.sh_info, liste.tab[i].sec.sh_addralign);
	}

	/* En termes de flags, certains n'étaient pas présents dans la documentation.
	   W, A, X sont dans la documentation chapitre 1-13 et 1-14 Figure 1-11.
	   Les autres flags sont dans la fonction reference readelf -S <nomFichier> */
	printf("Key to Flags:\n");
	printf("  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),\n");
	printf("  L (link order), O (extra OS processing required), G (group), T (TLS),\n");
	printf("  C (compressed), x (unknown), o (OS specific), E (exclude),\n");
	printf("  l (large), p (processor specific)\n");
}
