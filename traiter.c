
/* -- Copyright (c) 1990 - 1994 Inria/CNRS  All rights reserved. -- */

/*                    */
/* traiter.c          */
/* H. Richy            Juin 1991    */
/* adapte a la coupure Janvier 1993 */
/* coupure supprimee en mai 1994 */

/* preparation du dictionnaire pour chargement rapide dans Thot */
/* construction de la structure dictionnaire */
/* le fichier d'origine porte un suffixe .DIC */
/* le fichier pretraite produit porte un suffixe .dic */

#include "thot_sys.h"

#include "language.h"

/* constantes */
#include "constmedia.h"
#include "constmot.h"
#include "constcorr.h"

/* types */
#include "typemedia.h"
#include "typecorr.h"

#define MAXLIGNE   80		/* longueur d'une ligne dans le dictionnaire */

static unsigned char Code[256];	/* code des caracteres de l'alphabet */
static PtrDico      pdict;

#ifdef __STDC__
/* le dictionnaire */

/*********************************** init_code *****************************/
void                init_code (FILE * falpha)

#else  /* __STDC__ */
/* le dictionnaire */

/*********************************** init_code *****************************/
void                init_code (falpha)
FILE               *falpha;

#endif /* __STDC__ */

{
   unsigned char       x;
   int                 i;

   for (i = 0; i < 256; i++)
      Code[i] = 100;

   i = 1;
   while ((fscanf (falpha, "%c", &x) != EOF) && (i < NbLtr))
     {
	Code[x] = i;
	i++;
     }
   close (falpha);
}

/***************************** ALPHABET ************************************/

#ifdef __STDC__
boolean             alphabet ()

#else  /* __STDC__ */
boolean             alphabet ()
#endif				/* __STDC__ */

{
   FILE               *falpha;

   boolean             ret = False;

   if ((falpha = fopen ("alphabet", "r")) != NULL)
     {
	ret = True;
	init_code (falpha);
     }
   return ret;
}

/******************************** pretraitement *****************************/
/* 
   Pretraitement du dictionnaire pour determiner les lettres
   communes entre deux mots consecutifs
   (copie de la procedure pretraitement de corrcalcul.c)
 */

#ifdef __STDC__
void                pretraitement (PtrDico dico)

#else  /* __STDC__ */
void                pretraitement (dico)
PtrDico             dico;

#endif /* __STDC__ */

{
   int                 mot, i;
   char                dernier_mot[Lg_Mot];
   char                mot_courant[Lg_Mot];

   printf ("******* Debut du traitement ...\n");
   /* ne pas chercher a faire le pretraitement sur un dico vide */
   if (dico->nbmots >= 0)
     {
	dernier_mot[0] = 0;
	mot_courant[0] = 0;

	for (mot = 0; mot < dico->nbmots; mot++)
	  {
	     int                 k = 0;

	     strcpy (dernier_mot, mot_courant);
	     strcpy (mot_courant, &dico->chaine[dico->pdico[mot]]);

	     if (strlen (dernier_mot) != strlen (mot_courant))
	       {
		  /* changement de taille de mot */
		  /* => pas calcul de lettres communes */
		  dico->commun[mot] = 1;
	       }
	     else
	       {
		  /* recherche des lettres communes entre deux mots consecutifs */
		  /* afin de ne pas refaire des calculs */
		  while (mot_courant[k] == dernier_mot[k])
		     k++;
		  dico->commun[mot] = k + 1;
	       }
	  }
     }
   for (i = mot; i < dico->MAXmots; i++)
      dico->commun[i] = 1;	/* pour finir en beaute !!! */

   printf ("****** ...fin du traitement.\n");
}				/* end of pretraitement */

/****************************** charger ***************/
/*  retourne 1 si le chargement est reussi ou 0 sinon */
/* (copie de la procedure charger de corrcalcul.c     */
/******************************************************/

#ifdef __STDC__
int                 charger (FILE * fdico, PtrDico dict)

#else  /* __STDC__ */
int                 charger (fdico, dict)
FILE               *fdico;
PtrDico             dict;

#endif /* __STDC__ */

{
   char                motlu[Lg_Mot];
   char                lignelue[MAXLIGNE];
   char               *plignelue;
   int                 i, k, Taille, nblu, lglue, dermot;
   int                 Max_Car, Max_Mot, ptlu;
   int                 TailleCourante = 0;
   int                 nbcar = 0;	/* Nb de caracteres du dictionnaire */

   printf ("********** Debut du chargement...\n");

   dict->nbmots = -1;		/* le premier mot aura l'indice 0 */
   dict->chaine[0] = '\0';
   Max_Car = dict->MAXcars;	/* compte-tenu de l'allocation dynamique */
   Max_Mot = dict->MAXmots;	/* compte-tenu de l'allocation dynamique */
   plignelue = &lignelue[0];	/* pointeur sur le premier caractere lu */

   /* Chargement du dico */
   while (fgets (plignelue, MAXLIGNE, fdico) != NULL)
     {
	nblu = sscanf (plignelue, "%s", motlu);
	if ((nblu > 0)
	    && (dict->nbmots < Max_Mot - 1)
	    && ((Taille = strlen (motlu)) < Lg_Mot)
	    && (Taille + nbcar + 1 < Max_Car - 1))
	  {
	     dict->nbmots++;
	     plignelue = plignelue + Taille;
	     dict->pdico[dict->nbmots] = nbcar;
	     if (Taille != TailleCourante)
	       {
		  for (k = TailleCourante + 1; k <= Taille; k++)
		     dict->plgdico[k] = dict->nbmots;
		  TailleCourante = Taille;
	       }
	     for (k = 0; k < Taille; k++)
		dict->chaine[nbcar++] = Code[(unsigned char) motlu[k]];
	     dict->chaine[nbcar++] = '\0';	/* marqueur de fin de mot */

	     /* passer au mot suivant: lire ligne suivante */
	     plignelue = &lignelue[0];	/* pointeur sur le premier caractere lu */
	  }			/* end of if */
	else
	  {
	     if (nblu != -1)	/* ce n'est pas la fin du dico */
	       {
		  /* impossible de charger ce dictionnaire */
		  fclose (fdico);
		  printf ("  ERREUR etrange: dictionaire plus grand que prevu \n");
		  /* il faudrait liberer le dico */
		  return (0);
	       }		/* end of if */
	  }			/* end of else */
     }				/* end of while */

   /* creation d'un mot vide a la fin du dictionnaire */
   dermot = dict->nbmots + 1;
   dict->pdico[dermot] = nbcar;
   /* mise a jour des pointeurs */
   for (i = TailleCourante + 1; i < Lg_Mot; i++)
      dict->plgdico[i] = dermot;
   dict->chaine[nbcar] = '\0';
   dict->nbcars = nbcar;
   dict->DicoCharge = True;

   printf ("********...fin du chargement.\n");

   fclose (fdico);
   return (1);
}

/****************************** vider *************************************/

#ifdef __STDC__
void                vider (PtrDico dict, char *nomdict)

#else  /* __STDC__ */
void                vider (dict, nomdict)
PtrDico             dict;
char               *nomdict;

#endif /* __STDC__ */

{
   FILE               *fdico;	/* Fichier dictionnaire .dic */
   int                 nbcars, nbmots;

/*  SAUVER  nbmots, nbcars, chaine, commun, pdico, plgdico, */

   if (dict->nbmots >= 0)
     {
	fdico = fopen (nomdict, "w");
	/* pour indiquer la place necessaire pour ce dico (dans Thot) */
	nbmots = dict->nbmots;
	nbcars = dict->nbcars;
	fwrite (&nbmots, sizeof (int), 1, fdico);
	fwrite (&nbcars, sizeof (int), 1, fdico);

	fwrite (dict->chaine, nbcars, 1, fdico);
	fwrite (dict->commun, nbmots, 1, fdico);
	fwrite (dict->pdico, sizeof (int), nbmots, fdico);
	fwrite (dict->plgdico, sizeof (int), Lg_Mot, fdico);

	fclose (fdico);
     }
}				/* end of vider */

/****************************** creer *************************************/

#ifdef __STDC__
void                creer (PtrDico dict, char *nomdict)

#else  /* __STDC__ */
void                creer (dict, nomdict)
PtrDico             dict;
char               *nomdict;

#endif /* __STDC__ */

{
   FILE               *fdico;	/* Fichier dictionnaire .dic */

/*  LIRE  nbmots, nbcars, chaine, commun, pdico, plgdico, */
   fdico = fopen (nomdict, "r");
   fread (&dict->nbmots, sizeof (int), 1, fdico);
   fread (&dict->nbcars, sizeof (int), 1, fdico);

   fread (dict->chaine, dict->nbcars, 1, fdico);
   fread (dict->commun, dict->nbmots, 1, fdico);
   fread (dict->pdico, sizeof (int), dict->nbmots, fdico);
   fread (dict->plgdico, sizeof (int), Lg_Mot, fdico);

   fclose (fdico);
}				/* end of creer */

/****************************** memdico ******************************/
/* initialise le dictionnaire a 0 */
/*******************************************************************/

#ifdef __STDC__
int                 memdico ()

#else  /* __STDC__ */
int                 memdico ()
#endif				/* __STDC__ */

{
   int                 i;

   pdict = (PtrDico) malloc (sizeof (Dictionnaire));
   if (pdict == NULL)
      return (0);
   else
     {
	pdict->chaine = NULL;
	pdict->pdico = NULL;
	pdict->commun = NULL;
	for (i = 0; i < Lg_Mot; i++)
	   pdict->plgdico[i] = 0;
	pdict->MAXmots = 0;
	pdict->MAXcars = 0;
	pdict->nbmots = 0;
	pdict->nbcars = 0;
	return (1);
     }				/* end of else */
}				/* end of memdico */

/*CORR */

#ifdef __STDC__
void                GetStringInDict ()

#else  /* __STDC__ */
void                GetStringInDict ()
#endif				/* __STDC__ */

{
/*CORR */ unsigned int i;

/*CORR */
   pdict->MAXcars += 2;
/*CORR */ i = pdict->MAXcars;
   /*CORR *//* alloue la chaine necessaire */
/*CORR */ pdict->chaine = (PtrChaine) malloc (i);
/*CORR */
   pdict->MAXmots += 2;
/*CORR */ i = pdict->MAXmots;
/*CORR */ pdict->commun = (PtrCommuns) malloc (i);
/*CORR */
   /*CORR *//* ATTENTION : ce sont des entiers => 4 * i */
/*CORR */ pdict->pdico = (PtrMots) malloc (4 * i);

/*CORR */ 
}

/*************************************************************************/

#ifdef __STDC__
int                 main (int argc, char *argv[])

#else  /* __STDC__ */
int                 main (argc, argv)
int                 argc;
char               *argv[];

#endif /* __STDC__ */

{
   char                nomdico[Lg_Mot], nom[Lg_Mot];
   FILE               *fichdico;
   int                 i;

   if (alphabet () != 0)	/* initialisation de Code */
     {
	if (argc == 2)
	  {
	     strncpy (nomdico, argv[1], Lg_Mot);
	     strcpy (nom, nomdico);
	     strcat (nomdico, ".DIC");

	     /* dans toutes ces procedures le dictionnaire est pdict */
	     if ((fichdico = fopen (nomdico, "r")) != NULL)
	       {
		  /* alloue l'espace pour le dictionnaire */
		  if (memdico () != 1)
		    {
		       printf (" ERREUR: taille memoire insuffisante\n");
		       exit (1);
		    }
		  else
		    {
		       int                 im, ic, ip;

		       strcpy (pdict->DicoNom, nomdico);
		       /* alloue l'espace pour chaine, commun et pdico */
		       if (fscanf (fichdico, "%d%d%d", &im, &ic, &ip) != EOF)
			 {
			    pdict->MAXmots = im;
			    pdict->MAXcars = ic;
			    GetStringInDict ();
			    printf ("*** Lecture du dictionnaire %s\n", nomdico);
			    charger (fichdico, pdict);
			    pretraitement (pdict);
			    strcat (nom, ".dic");
			    printf ("***** Ecriture dans le dictionnaire %s\n", nom);
			    vider (pdict, nom);
			    printf ("**** Relecture pour verification \n");
			    creer (pdict, nom);
			    printf ("*** nbmots = %i,  nbcars = %i", pdict->nbmots, pdict->nbcars);
/* TRACE PROVISOIRE */
			    printf ("- MAXmots = %i,  MAXcars = %i",
				    pdict->MAXmots, pdict->MAXcars);
			    printf ("- fin de chaine %c\n", pdict->chaine[pdict->nbcars - 1]);
			    printf ("- pdico de nbmots %i\n", pdict->pdico[pdict->nbmots]);
			    printf ("- plgdico de nbmots %i\n", pdict->plgdico[pdict->nbmots]);
/* END OF TRACE */
			    printf ("*     C'EST BON *********\n");
			 }	/* end of fscanf */
		       else
			  printf ("  ERREUR: le dictionnaire %s est vide", nomdico);
		    }
	       }
	     else
		printf ("  ERREUR: le dictionnaire %s est inaccessible\n", nomdico);
	  }
	else
	   /* ajouter le nom de fichier en parametre */
	   printf ("  ERREUR: il fallait indiquer un nom de fichier dictionnaire\n");
     }
   else
      printf ("  ERREUR: le fichier alphabet est inaccessible\n");
}				/* end of main */
