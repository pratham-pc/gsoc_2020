/*
 * synonyms_impl.c: Implementation of synonyms functions
 *
 * Before modifying this file read README.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "synonyms_impl.h"

typedef struct _WordNode WordNode;
typedef struct _SynonymListNode SynonymListNode;

#define TABLE_SIZE 100

struct _SynonymListNode {
    WordNode *w;
    SynonymListNode *next;
};


struct _WordNode {
    char *word;
    WordNode *parent;
    WordNode *next;
    bool isRepresentative;
    int rank;
    SynonymListNode *listHead;
    SynonymListNode *listTail;
};

struct _Synonyms {
    WordNode *table[TABLE_SIZE];
};


/**
 * synonyms_init:
 *
 * Returns: an instance of synonyms dictionary, or
 *          NULL on error (with errno set properly).
 */
Synonyms *
synonyms_init(void)
{
    Synonyms* s = (Synonyms *)malloc(sizeof(Synonyms));
    memset(s, 0, sizeof(Synonyms));
    return s;
}


/**
 * wordnode_free:
 * @w: instance of wordnode
 *
 * Frees previously allocated wordnode
 */
void
wordnode_free(WordNode *w)
{
    if (!w)
        return;
    free(w->word);
    if (w->isRepresentative) {
        SynonymListNode *s = w->listHead;
        while (s) {
            SynonymListNode *t = s;
            s = s->next;
            free(t);
        }
    }
    free(w);
}


/**
 * synonyms_free:
 * @s: instance of synonyms dictionary
 *
 * Frees previously allocated dictionary. If @s is NULL then this
 * is NO-OP.
 */
void
synonyms_free(Synonyms *s)
{
    int i;
    if (!s)
        return;
    for (i = 0; i < TABLE_SIZE; i++) {
        WordNode *w = s->table[i];
        while (w) {
            WordNode *t = w;
            w = w->next;
            wordnode_free(t);
        }
    }
    free(s);
}


/*
 * getHashVal:
 * @word: string of word
 *
 * Hash function to map word to WordNodes
 * in Synonym struct
 */
int
getHashVal(const char *word)
{
    int i;
    long long int val = 0;
    for (i = 0; i < strlen(word); i++) {
        val *= 26;
        val += word[i];
        val %= 100;
    }
    return val;
}


/*
 * FindWordNode:
 * @s: Synonyms struct
 * @word: string of word
 *
 * Find given word's WordNode in Synonym struct
 */
WordNode *
FindWordNode(Synonyms *s, const char *word)
{
    int i = getHashVal(word);
    WordNode *w = s->table[i];
    while (w) {
        if (!(strcmp(word, w->word))) {
            return w;
        }
        w = w->next;
    }
    return NULL;
}


/*
 * GetNewWordNode:
 * @word: string of word
 *
 * Generates a new WordNode
 */
WordNode *
GetNewWordNode(const char *word)
{
    WordNode *w = (WordNode *)malloc(sizeof(WordNode));
    memset(w, 0, sizeof(WordNode));
    w->word = malloc(strlen(word)+1);
    memset(w->word, 0, strlen(word)+1);
    strncpy(w->word, word, strlen(word));
    w->word[strlen(word)] = '\0';
    w->isRepresentative = true;
    return w;
}


/*
 * InsertWordNode:
 * @word: string of word
 *
 * insert word in the synonyms struct
 */
WordNode *
InsertWordNode(Synonyms *s, const char *word)
{
    int i = getHashVal(word);
    WordNode *w = s->table[i];
    if (!w) {
        s->table[i] = GetNewWordNode(word);
        return s->table[i];
    }
    while (w->next) {
        w = w->next;
    }
    w->next = GetNewWordNode(word);
    return w->next;
}


/*
 * GetRepresentative:
 * @w: WordNode
 *
 * Generates the Representative(parent) of given wordnode
 */
WordNode *
GetRepresentative(WordNode *w)
{
    while (!w->isRepresentative) {
        w = w->parent;
    }
    return w;
}


/*
 * GetNewSynonymListNode:
 * @w: WordNode
 *
 * Generates the SynonymListNode for a new Node
 */
SynonymListNode *
GetNewSynonymListNode(WordNode *w)
{
    SynonymListNode *s = (SynonymListNode *)malloc(sizeof(SynonymListNode));
    s->w = w;
    s->next = NULL;
    return s;
}


/*
 * UpdateParent:
 * @s: SynonymListNode
 * @p: WordNode
 *
 * Marks all the word's in the list of SynonymListNodes
 * parent as p
 */
void
UpdateParent(SynonymListNode *s, WordNode *p)
{
    while (s) {
        s->w->parent = p;
        s = s->next;
    }
}


/*
 * AddSynonyms:
 * @p1: WordNode
 * @p2: WordNode
 *
 * Merges synonym sets of wordnode p1 and p2
 */
WordNode *
AddSynonyms(WordNode *p1, WordNode *p2)
{
    if (p1 == p2)
        return p1;
    if(p1->rank >= p2->rank) {
        p2->parent = p1;
        p2->isRepresentative = false;
        p1->rank += p2->rank + 1;
        p2->rank = 0;
        UpdateParent(p2->listHead, p1);
        if (p1->listHead == NULL) {
            p1->listHead = GetNewSynonymListNode(p2);
            p1->listTail = p1->listHead;
        } else {
            p1->listTail->next = GetNewSynonymListNode(p2);
            p1->listTail = p1->listTail->next;
            p1->listTail->next = p2->listHead;
            if (p2->listTail)
                p1->listTail = p2->listTail;
        }
        p2->listHead = NULL;
        p2->listTail = NULL;
        return p1;
    } else {
        p1->parent = p2;
        p1->isRepresentative = false;
        p2->rank += p1->rank + 1;
        p1->rank = 0;
        UpdateParent(p1->listHead, p2);
        if (p2->listHead == NULL) {
            p2->listHead = GetNewSynonymListNode(p1);
            p2->listTail = p2->listHead;
        } else {
            p2->listTail->next = GetNewSynonymListNode(p1);
            p2->listTail = p2->listTail->next;
            p2->listTail->next = p1->listHead;
            if (p1->listTail)
                p2->listTail = p1->listTail;
        }
        p1->listHead = NULL;
        p1->listTail = NULL;
        return p2;
    }
}

/**
 * synonyms_define:
 * @s: instance of synonyms dictionary
 * @word: a word to add to the dictionary
 * @args: a list of synonyms
 *
 * For given @word, add it to the dictionary and define its synonyms. If the
 * @word already exists in the dictionary then just extend its list of
 * synonyms.
 *
 * Returns 0 on success, -1 otherwise.
 */
int
synonyms_define(Synonyms *s,
                const char *word, ...)
{
    WordNode *w1 = FindWordNode(s, word);
    if (!w1) {
        w1 = InsertWordNode(s, word);
    }
    w1 = GetRepresentative(w1);
    va_list word_list;
    va_start(word_list, word);
    const char *t = va_arg(word_list, const char*);
    while (t) {
        WordNode *w2 = FindWordNode(s, t);
        if (!w2) {
            w2 = InsertWordNode(s, t);
        }
        w2 = GetRepresentative(w2);
        w1 = AddSynonyms(w1, w2);
        t = va_arg(word_list, const char*);
    }
    va_end(word_list);
    return 0;
}


/*
 * is_synonym:
 * @s: instance of synonyms dictionary
 * @w1: a word
 * @w2: a word
 *
 * Checks whether @w1 is defined synonym of @w2 (or vice versa).
 */
bool
is_synonym(Synonyms *s,
           const char *w1,
           const char *w2)
{
    if (!strcmp(w1, w2))
        return false;
    WordNode *W1 = FindWordNode(s, w1);
    WordNode *W2 = FindWordNode(s, w2);
    if (!W1 || !W2) {
        return false;
    }

    if (!W1->parent) {
        return W1 == W2->parent;
    } else if (!W2->parent) {
        return W2 == W1->parent;
    } else {
        return W1->parent == W2->parent;
    }
}


/**
 * synonyms_get:
 * @s: instance of synonyms dictionary
 * @word: a word
 *
 * Returns: a string list of defined synonyms for @word, or
 *          NULL if no synonym was defined or an error occurred.
 */
char **
synonyms_get(Synonyms *s,
             const char *word)
{
    char** synonym_list;
    int i;
    WordNode *w = FindWordNode(s, word);
    w = GetRepresentative(w);
    synonym_list = (char **)malloc(sizeof(char *)*(w->rank));
    if (!(strcmp(w->word, word))) {
        SynonymListNode *s = w->listHead;
        i = 0;
        while (i < w->rank) {
            synonym_list[i] = (char *)malloc(strlen(s->w->word)+1);
            memcpy(synonym_list[i], s->w->word, strlen(s->w->word)+1);
            s = s->next;
            i++;
        }
    } else {
        synonym_list[0] = (char *)malloc(strlen(w->word)+1);
        memcpy(synonym_list[0], w->word, strlen(w->word)+1);
        SynonymListNode *s = w->listHead;
        i = 1;
        while (i < w->rank) {
            if (!(strcmp(s->w->word, w->word))) {
                s = s->next;
                continue;
            }
            synonym_list[i] = (char *)malloc(strlen(s->w->word)+1);
            memcpy(synonym_list[i], s->w->word, strlen(s->w->word)+1);
            s = s->next;
            i++;
        }
    }

    return synonym_list;
}
