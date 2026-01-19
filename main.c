#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- CONSTANTES ET STRUCTURES ---

typedef struct liste {
    char mot[100];
    int taille;
    int ligne;
    struct liste *next;
} liste;

typedef struct {
    char nom[50];
    char type[20];
} Symbole;

// Table des symboles globale (utilisée par Syntaxe et Sémantique)
Symbole tableSymboles[100];
int nbSymboles = 0;
int lastLigne;
// --- FONCTIONS UTILITAIRES ---

// Gestion des erreurs (Console + Fichier)
void rapportErreur(const char *type, int ligne, const char *message, int *flagErreur) {
    FILE *errors = fopen("errors.txt", "a+");
    printf("%s (Ligne %d) : %s\n\n", type, ligne, message);
    if (errors) {
        fprintf(errors, "%s (Ligne %d) : %s\n", type, ligne, message);
        fclose(errors);
    }
    if (flagErreur) *flagErreur = 1;
}

// Vérifie si une chaîne est une variable valide (A-Z)
int estNomVariable(char *mot) {
    return (strlen(mot) == 1 && ((mot[0] >= 'A' && mot[0] <= 'Z') || (mot[0] >= 'a' && mot[0] <= 'z')));
}

// Vérifie si un mot existe dans un fichier (Dictionnaire)
int motDansFichier(char *mot, char *cheminFichier) {
    FILE *file = fopen(cheminFichier, "r");
    if (!file) return 0;
    
    char buffer[100];
    while (fscanf(file, "%s", buffer) != EOF) {
        if (strcmp(mot, buffer) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

// Verifier si le symbole existe dans le tableau
int getIndiceSymbole(char *nom) {
    for (int i = 0; i < nbSymboles; i++) {
        if (strcmp(tableSymboles[i].nom, nom) == 0) return i;
    }
    return -1;
}

// Ajout de symbole a la table
void ajouterSymbole(char *nom, char *type) {
    if (getIndiceSymbole(nom) == -1) { // Si le symbole n'existe pas dans la table
        strcpy(tableSymboles[nbSymboles].nom, nom);
        strcpy(tableSymboles[nbSymboles].type, type);
        nbSymboles++;
    }
}

// Avoir le type d'un symbole
char* getTypeSymbole(char *nom) {
    int i = getIndiceSymbole(nom); // Verifier si le symbole existe dans la table
    return (i != -1) ? tableSymboles[i].type : NULL;
}

// --- GESTION DE LISTE CHAINEE ---

// Creer et initialiser une liste
liste *creerliste(char mot[], int taille, int ligne) {
    liste* nouveau = (liste*) malloc(sizeof(liste));
    strcpy(nouveau->mot, mot);
    nouveau->taille = taille;
    nouveau->ligne = ligne;
    nouveau->next = NULL;
    return nouveau;
}

// Ajouter un mot a la fin de la liste
liste* ajouterFin(liste* head, char mot[], int taille, int ligne) {
    if (head == NULL) return creerliste(mot, taille, ligne);
    
    liste *current = head;
    while (current->next != NULL) current = current->next;
    current->next = creerliste(mot, taille, ligne);
    return head;
}

// Affichage de la liste
void afficherListe(liste *head) {
    if (!head) { printf("La liste est vide.\n"); return; }
    printf("%-20s %-10s %-10s\n", "MOT", "TAILLE", "LIGNE");
    printf("--------------------------------------------\n");
    while (head) {
        printf("%-20s %-10d %-10d\n", head->mot, head->taille, head->ligne);
        head = head->next;
    }
}

// --- CONSTRUCTION (LEXER BASE) ---

// Construire la liste chainee a partir du fichier
liste* construction(liste *head, FILE *p) {
    char mot[100] = "";
    int ligne = 1;
    char c;

    while ((c = fgetc(p)) != EOF) {
        if (c == ' ' || c == '\n' || c == '\t') {
            if (strlen(mot) > 0) {
                head = ajouterFin(head, mot, strlen(mot), ligne);
                mot[0] = '\0';
            }
            if (c == '\n') ligne++;
        } 
        else if (strchr("();>:=.", c)) { // Caractères spéciaux simples
            if (strlen(mot) > 0) {
                head = ajouterFin(head, mot, strlen(mot), ligne);
                mot[0] = '\0';
            }
            char special[2] = {c, '\0'};
            head = ajouterFin(head, special, 1, ligne);
        } 
        else if (c == '"') { // Gestion des chaînes de caractères
            if (strlen(mot) > 0) {
                head = ajouterFin(head, mot, strlen(mot), ligne);
                mot[0] = '\0';
            }
            head = ajouterFin(head, "\"", 1, ligne);
            while ((c = fgetc(p)) != EOF && c != '"') {
                int len = strlen(mot);
                if (len < 99) { mot[len] = c; mot[len+1] = '\0'; }
            }
            if (strlen(mot) > 0) {
                head = ajouterFin(head, mot, strlen(mot), ligne);
                mot[0] = '\0';
            }
            head = ajouterFin(head, "\"", 1, ligne);
        } 
        else {
            int len = strlen(mot);
            if (len < 99) { mot[len] = c; mot[len+1] = '\0'; }
        }
    }
    lastLigne = ligne;
    if (strlen(mot) > 0) head = ajouterFin(head, mot, strlen(mot), ligne);
    return head;
}

// --- ANALYSE LEXICALE ---

void analyserListeLexicale(liste *head, char *dict) {
    liste *current = head;
    int modeChaine = 0;

    printf("--- Debut Analyse Lexicale ---\n");
    while (current != NULL) {
        if (strcmp(current->mot, "\"") == 0) {
            modeChaine = !modeChaine;
            current = current->next;
            continue;
        }

        if (!modeChaine) {
            // On ignore les nombres, les variables A-Z et la ponctuation pour le dico
            if (!estNomVariable(current->mot) && !isdigit(current->mot[0]) && !strchr("();>:=.", current->mot[0])) {
                if (!motDansFichier(current->mot, dict)) {
                    rapportErreur("Erreur Lexicale", current->ligne, "Mot inconnu dans le dictionnaire.", NULL);
                }
            }
        }
        current = current->next;
    }
    printf("Fin Analyse Lexicale.\n\n");
}

// --- ANALYSE SYNTAXIQUE ---

void analyseSyntaxique(liste *head) {
    char *dictionnaire = "dictionnaire.txt";
    liste *current = head;
    int err = 0; // Flag erreur global
    int siExist = 0;

    printf("--- Debut Analyse Syntaxique ---\n");

    // 1. Vérification du début
    if (strcmp(current->mot, "debut") != 0) {
        rapportErreur("Erreur Syntaxique", current->ligne, "Le programme doit commencer par 'debut'.", &err);
        return;
    }
    current = current->next;
    if (!current || strcmp(current->mot, ":") != 0) {
        rapportErreur("Erreur Syntaxique", current->ligne, "':' manquant apres 'debut'.", &err);
        return;
    }
    current = current->next;

    // Boucle principale
    while(current != NULL && !err) {
        int traite = 0;

        // --- Bloc LIRE / ECRIRE ---
        EtiquetteInstruction: // Point de retour pour les instructions imbriquées
        if (strcmp(current->mot, "Lire") == 0 || strcmp(current->mot, "Ecrire") == 0) {
            traite = 1;
            current = current->next;
            if (strcmp(current->mot, "(") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "'(' manquant.", &err); break; }
            current = current->next;

            // Gestion contenu (Variable ou Chaine)
            if (strcmp(current->mot, "\"") == 0) {
                current = current->next; // On passe le guillemet
                // On accepte le contenu texte...
                if (current->next && strcmp(current->next->mot, "\"") == 0) current = current->next; // fin texte
                else { rapportErreur("Erreur Syntaxique", current->ligne, "'\"' fermant manquant.", &err); break; }
                current = current->next; // On passe le guillemet fermant
            } else if (estNomVariable(current->mot)) {
                current = current->next;
            } else {
                rapportErreur("Erreur Syntaxique", current->ligne, "Contenu Lire/Ecrire invalide.", &err); break;
            }

            if (strcmp(current->mot, ")") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "')' manquant.", &err); break; }
            current = current->next;
            if (strcmp(current->mot, ";") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "';' manquant.", &err); break; }
            
            // Logique pour enchaîner les instructions dans un SI
            if (current->next && (strcmp(current->next->mot, "Lire") == 0 || strcmp(current->next->mot, "Ecrire") == 0)) {
                current = current->next;
                goto EtiquetteInstruction;
            }
        }

        // --- Bloc DECLARATION (var A : Type) ---
        else if (strcmp(current->mot, "var") == 0) {
            traite = 1;
            current = current->next;
            
            char *nomVar = current->mot;
            if (!estNomVariable(nomVar)) { rapportErreur("Erreur Syntaxique", current->ligne, "Nom de variable invalide (A-Z).", &err); break; }
            
            current = current->next;
            if (strcmp(current->mot, ":") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "':' manquant.", &err); break; }
            
            current = current->next;
            char *typeVar = current->mot;
            if (strcmp(typeVar, "Entier") != 0 && strcmp(typeVar, "Reel") != 0) {
                rapportErreur("Erreur Syntaxique", current->ligne, "Type inconnu (Entier/Reel attendu).", &err); break;
            }

            // AJOUT TABLE DES SYMBOLES ICI (Pour usage ultérieur)
            ajouterSymbole(nomVar, typeVar);

            current = current->next;
            if (strcmp(current->mot, ";") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "';' manquant.", &err); break; }
        }

        // --- Bloc AFFECTATION (A = B) ---
        else if (estNomVariable(current->mot)) {
            // Vérification existence (Syntaxe contextuelle)
            if (getIndiceSymbole(current->mot) != -1) {
                traite = 1;
                char *nomGauche = current->mot;
                current = current->next;

                if (strcmp(current->mot, "=") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "'=' manquant.", &err); break; }
                current = current->next;

                // Partie Droite (Variable ou Nombre)
                if (getIndiceSymbole(current->mot) != -1 || isdigit(current->mot[0])) {
                    current = current->next;
                } else {
                    rapportErreur("Erreur Semantique", current->ligne, "Variable non declaree ou valeur invalide a droite.", &err); break;
                }

                if (strcmp(current->mot, ";") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "';' manquant.", &err); break; }
            } 
        }

        // --- Bloc SI ---
        else if (strcmp(current->mot, "Si") == 0) {
            siExist = 1;
            traite = 1;
            current = current->next;
            if (strcmp(current->mot, "(") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "'(' manquant.", &err); break; }
            
            // Condition simple : A operateur B
            current = current->next; // Var 1
            if (!estNomVariable(current->mot)) { rapportErreur("Erreur Syntaxique", current->ligne, "Comparant 1 invalide.", &err); break; }
            
            current = current->next; // Opérateur
            if (!strchr("<>!=", current->mot[0])) { rapportErreur("Erreur Syntaxique", current->ligne, "Operateur invalide.", &err); break; }
            
            current = current->next; // Var 2
            if (!estNomVariable(current->mot)) { rapportErreur("Erreur Syntaxique", current->ligne, "Comparant 2 invalide.", &err); break; }
            
            current = current->next;
            if (strcmp(current->mot, ")") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "')' manquant.", &err); break; }
            
            current = current->next;
            if (strcmp(current->mot, "Alors") != 0) { rapportErreur("Erreur Syntaxique", current->ligne, "'Alors' manquant.", &err); break; }
            
            // Suite des instructions...
            current = current->next;
            goto EtiquetteInstruction; 
        }

        // --- Bloc SINON ---
        else if (strcmp(current->mot, "Sinon") == 0) {
            if (!siExist) { rapportErreur("Erreur Syntaxique", current->ligne, "'Sinon' sans 'Si'.", &err); break; }
            traite = 1;
            current = current->next;
            goto EtiquetteInstruction;
        }

        // --- Bloc FINSI ---
        else if (strcmp(current->mot, "FinSi") == 0) {
            if (!siExist) { rapportErreur("Erreur Syntaxique", current->ligne, "'FinSi' sans 'Si'.", &err); break; }
            traite = 1;
            siExist = 0; // Reset
        }

        // --- Bloc FIN PROGRAMME ---
        else if (strcmp(current->mot, "Fin") == 0) {
            current = current->next;
            if (!current || strcmp(current->mot, ".") != 0) {
                rapportErreur("Erreur Syntaxique", current->ligne, "'.' manquant apres 'Fin'.", &err);
                err = 1;
            }
            break; // Sortie boucle
        } else if(current->ligne == lastLigne && strcmp(current->mot, "Fin") != 0) {
                rapportErreur("Erreur Syntaxique", current->ligne, "Le programme doit se terminer par 'Fin'.", &err);
        }

        // Avancer si non traité (Erreur) ou traité (Passer au suivant si pas géré par goto)
        if (!traite && !err) {
             rapportErreur("Erreur Syntaxique", current->ligne, "Instruction inconnue.", &err);
             break;
        }
        if (current) current = current->next;
    }

    if (!err) printf("Analyse Syntaxique reussie.\n\n");
    else printf("Echec Analyse Syntaxique.\n\n");
}

// --- ANALYSE SEMANTIQUE ---

void analyseSemantique(liste *head) {
    liste *current = head;
    int err = 0;

    printf("--- Debut Analyse Semantique ---\n");

    // On parcourt juste pour vérifier les affectations
    // La table des symboles a DEJA été remplie par l'analyse syntaxique
    while (current != NULL) {
        
        // Détection Affectation : A = B
        if (current->next != NULL && strcmp(current->next->mot, "=") == 0) {
            char *nomG = current->mot;
            char *nomD = current->next->next->mot;

            char *typeG = getTypeSymbole(nomG);
            char *typeD = getTypeSymbole(nomD); // Peut être NULL si c'est un nombre

            // Si c'est Variable = Variable
            if (typeG && typeD) {
                if (strcmp(typeG, "Entier") == 0 && strcmp(typeD, "Reel") == 0) {
                    char msg[100];
                    sprintf(msg, "Impossible d'affecter Reel (%s) vers Entier (%s).", nomD, nomG);
                    rapportErreur("Erreur Semantique", current->ligne, msg, &err);
                }
            }
            // Si c'est Variable = Nombre (ex: 5.5)
            else if (typeG && !typeD && strchr(nomD, '.')) {
                 if (strcmp(typeG, "Entier") == 0) {
                    rapportErreur("Erreur Semantique", current->ligne, "Impossible d'affecter une valeur decimale a un Entier.", &err);
                 }
            }
        }
        current = current->next;
    }

    if (!err) printf("Analyse Semantique reussie.\n\n");
    else printf("Echec Analyse Semantique.\n\n");
}

// --- MAIN ---

int main() {
    char *cheminProgramme = "programme.txt";
    char *dictionnaire = "dictionnaire.txt";

    FILE *p = fopen(cheminProgramme, "r");
    if (!p) { printf("Erreur : Impossible d'ouvrir %s.\n", cheminProgramme); return 1; }

    liste *head = NULL;
    head = construction(head, p);
    fclose(p);

    if (head) {
        afficherListe(head);
        analyserListeLexicale(head, dictionnaire);
        analyseSyntaxique(head);   // Remplit aussi la table des symboles
        analyseSemantique(head);   // Vérifie les types
    }

    return 0;
}