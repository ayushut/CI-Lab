/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/

#include "ci.h"

extern bool is_binop(token_t);
extern bool is_unop(token_t);
char *strrev(char *str);

/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr)
{
    if (nptr == NULL)
        return;
    // check running status
    if (terminate || ignore_input)
        return;
    if (nptr->node_type == NT_LEAF)
    {
        if(nptr->type == ID_TYPE){
            entry_t* ent = get(nptr->val.sval);
            nptr->type = ent->type;
            if(nptr->type == INT_TYPE){
                nptr->val.ival = ent->val.ival;
            }else{
                nptr->val.sval = (char *) malloc(strlen(nptr->val.sval)+1);
                strcpy(nptr->val.sval, ent->val.sval);
            }
        }
        return;
    }
    if (nptr->type == ID_TYPE)
    {
        infer_type(nptr->children[1]);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[2] != NULL)
        {
            if (nptr->children[0]->type != BOOL_TYPE || nptr->children[1]->type != nptr->children[2]->type)
            {
                handle_error(ERR_TYPE);
            }
            else
            {
                nptr->type = nptr->children[1]->type;
            }
        }
        else if (nptr->children[1] != NULL)
        {
            switch (nptr->children[0]->type)
            {
            case INT_TYPE:
                if (nptr->children[1]->type != INT_TYPE)
                {
                    handle_error(ERR_TYPE);
                }
                else if (nptr->tok == TOK_PLUS || nptr->tok == TOK_BMINUS || nptr->tok == TOK_TIMES || nptr->tok == TOK_MOD || nptr->tok == TOK_DIV)
                {
                    nptr->type = INT_TYPE;
                }
                else if (nptr->tok == TOK_LT || nptr->tok == TOK_GT || nptr->tok == TOK_EQ)
                {
                    nptr->type = BOOL_TYPE;
                }
                else if (nptr->tok == TOK_UMINUS || nptr->tok == TOK_NOT)
                {
                    handle_error(ERR_SYNTAX);
                }
                else
                {
                    handle_error(ERR_TYPE);
                }
                break;
            case STRING_TYPE:
                if (nptr->children[1]->type == STRING_TYPE || nptr->children[1]->type == INT_TYPE)
                {
                    if(nptr->tok == TOK_PLUS){
                        if(nptr->children[1]->type == INT_TYPE){
                            handle_error(ERR_TYPE);
                        }else{
                            nptr->type = STRING_TYPE;
                        }
                    }
                    else if (nptr->tok == TOK_TIMES)
                    {
                        nptr->type = STRING_TYPE;
                    }
                    else if (nptr->tok == TOK_GT || nptr->tok == TOK_LT || nptr->tok == TOK_EQ)
                    {
                        nptr->type = BOOL_TYPE;
                    }
                    else if (nptr->tok == TOK_UMINUS || nptr->tok == TOK_NOT)
                    {
                        handle_error(ERR_SYNTAX);
                    }else{
                        handle_error(ERR_TYPE);
                    }
                }
                else
                {
                    handle_error(ERR_TYPE);
                }
                break;
            case BOOL_TYPE:
                if (nptr->children[1]->type != BOOL_TYPE)
                {
                    handle_error(ERR_TYPE);
                }
                else if (nptr->tok == TOK_AND || nptr->tok == TOK_OR)
                {
                    nptr->type = BOOL_TYPE;
                }
                else if (nptr->tok == TOK_UMINUS || nptr->tok == TOK_NOT)
                {
                    handle_error(ERR_SYNTAX);
                }
                else
                {
                    handle_error(ERR_TYPE);
                }
                break;
            default:
                handle_error(ERR_TYPE);
            }
        }
        else if (nptr->children[0] != NULL)
        {
            switch (nptr->children[0]->type)
            {
            case INT_TYPE:
                if (nptr->tok == TOK_UMINUS)
                {
                    nptr->type = INT_TYPE;
                }
                // else if (nptr->tok == TOK_NOT)
                // {
                //     handle_error(ERR_SYNTAX);
                // }
                else
                {
                    handle_error(ERR_TYPE);
                }
                break;
            case STRING_TYPE:
                if (nptr->tok == TOK_UMINUS)
                {
                    nptr->type = STRING_TYPE;
                }
                else if (nptr->tok == TOK_NOT)
                {
                    handle_error(ERR_SYNTAX);
                }
                else
                {
                    handle_error(ERR_TYPE);
                }
                break;
            case BOOL_TYPE:
                if (nptr->tok == TOK_NOT)
                {
                    nptr->type = BOOL_TYPE;
                }
                else if (nptr->tok == TOK_UMINUS)
                {
                    handle_error(ERR_SYNTAX);
                }
                else
                {
                    handle_error(ERR_TYPE);
                }
                break;
            default:
                handle_error(ERR_TYPE);
            }
        }
        else
        {
            handle_error(ERR_TYPE);
        }
        return;
    }
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated. 
 */

static void infer_root(node_t *nptr)
{
    if (nptr == NULL)
        return;
    // check running status
    if (terminate || ignore_input)
        return;

    // check for assignment
    if (nptr->type == ID_TYPE)
    {
        infer_type(nptr->children[1]);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[0] == NULL)
        {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

/* eval_node() - set the value of a non-root node based on the values of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The val field of the node is updated.
 * (STUDENT TODO) 
 */

static void eval_node(node_t *nptr)
{
    if (nptr == NULL)
        return;
    // check running status
    if (terminate || ignore_input)
        return;

    if (nptr->node_type == NT_LEAF)
    {
        return;
    }

    eval_node(nptr->children[0]);
    switch (nptr->type)
    {
    case INT_TYPE:
        switch (nptr->tok)
        {
        case TOK_QUESTION:
            if (nptr->children[0]->val.bval)
            {
                eval_node(nptr->children[1]);
                nptr->val.ival = nptr->children[1]->val.ival;
            }
            else
            {
                eval_node(nptr->children[2]);
                nptr->val.ival = nptr->children[2]->val.ival;
            }
            break;
        case TOK_PLUS:
            eval_node(nptr->children[1]);
            nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival;
            break;
        case TOK_BMINUS:
            eval_node(nptr->children[1]);
            nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival;
            break;
        case TOK_TIMES:
            eval_node(nptr->children[1]);
            nptr->val.ival = nptr->children[0]->val.ival * nptr->children[1]->val.ival;
            break;
        case TOK_DIV:
            eval_node(nptr->children[1]);
            if(nptr->children[1]->val.ival == 0){
                handle_error(ERR_EVAL);
            }else{
                nptr->val.ival = nptr->children[0]->val.ival / nptr->children[1]->val.ival;
            }
            break;
        case TOK_MOD:
            eval_node(nptr->children[1]);
            if(nptr->children[1]->val.ival == 0){
                handle_error(ERR_EVAL);
            }else{
                nptr->val.ival = nptr->children[0]->val.ival % nptr->children[1]->val.ival;
            }
            break;
        case TOK_UMINUS:
            nptr->val.ival = nptr->children[0]->val.ival * -1;
            break;
        default:
            break;
        }
        break;
    case STRING_TYPE:
        switch (nptr->tok)
        {
        case TOK_QUESTION:
            if (nptr->children[0]->val.bval)
            {
                eval_node(nptr->children[1]);
                nptr->val.sval = (char *)malloc(strlen(nptr->children[1]->val.sval) + 1);
                strcpy(nptr->val.sval, nptr->children[1]->val.sval);
            }
            else
            {
                eval_node(nptr->children[2]);
                nptr->val.sval = (char *)malloc(strlen(nptr->children[2]->val.sval) + 1);
                strcpy(nptr->val.sval, nptr->children[2]->val.sval);
            }
            break;
        case TOK_PLUS:
            eval_node(nptr->children[1]);
            nptr->val.sval = (char *)malloc(strlen(nptr->children[1]->val.sval) + strlen(nptr->children[0]->val.sval) + 1);
            strcpy(nptr->val.sval, nptr->children[0]->val.sval);
            strcat(nptr->val.sval, nptr->children[1]->val.sval);
            break;
        case TOK_TIMES:
            eval_node(nptr->children[1]);
            char *empt = "";
            if (nptr->children[1]->val.ival == 0)
            {
                nptr->val.sval = empt;
            }
            else
            {
                nptr->val.sval = (char *)malloc((strlen(nptr->children[0]->val.sval) * nptr->children[1]->val.ival) + 1);
                strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                for (int i = 1; i < nptr->children[1]->val.ival; i++)
                {
                    strcat(nptr->val.sval, nptr->children[0]->val.sval);
                }
            }
            break;
        case TOK_UMINUS:
            nptr->val.sval = strrev(nptr->children[0]->val.sval);
            break;
        default:
            break;
        }
        break;

    case BOOL_TYPE:
        switch (nptr->tok)
        {
        case TOK_NOT:
            nptr->val.bval = !(nptr->children[0]->val.bval);
            break;
        case TOK_GT:
            eval_node(nptr->children[1]);
            if(nptr->children[0]->type == INT_TYPE){
                nptr->val.bval = (nptr->children[0]->val.ival > nptr->children[1]->val.ival);
            }else{
                nptr->val.bval = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) > 0);
            }
            break;
        case TOK_LT:
            eval_node(nptr->children[1]);
            if(nptr->children[0]->type == INT_TYPE){
                nptr->val.bval = (nptr->children[0]->val.ival < nptr->children[1]->val.ival);
            }else{
                nptr->val.bval = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) < 0);
            }
            break;
        case TOK_EQ:
            eval_node(nptr->children[1]);
            if(nptr->children[0]->type == INT_TYPE){
                nptr->val.bval = (nptr->children[0]->val.ival == nptr->children[1]->val.ival);
            }else{
                nptr->val.bval = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) == 0);
            }
            break;
        case TOK_AND:
            eval_node(nptr->children[1]);
            nptr->val.bval = (nptr->children[0]->val.bval && nptr->children[1]->val.bval);
            break;
        case TOK_OR:
            eval_node(nptr->children[1]);
            nptr->val.bval = (nptr->children[0]->val.bval || nptr->children[1]->val.bval);
            break;
        default:
            break;
        }

    default:
        break;
    }
    return;
}

/* eval_root() - set the value of the root node based on the values of children 
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated. 
 */

void eval_root(node_t *nptr)
{
    if (nptr == NULL)
        return;
    // check running status
    if (terminate || ignore_input)
        return;

    // check for assignment
    if (nptr->type == ID_TYPE)
    {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input)
            return;

        if (nptr->children[0] == NULL)
        {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i)
    {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input)
        return;

    if (nptr->type == STRING_TYPE)
    {
        (nptr->val).sval = (char *)malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (!nptr->val.sval)
        {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    }
    else
    {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

/* infer_and_eval() - wrapper for calling infer() and eval() 
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated. 
 */

void infer_and_eval(node_t *nptr)
{
    infer_root(nptr);
    eval_root(nptr);
    return;
}

/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str)
{
    char *revstr = malloc(strlen(str) + 1);
    int end = strlen(str) - 1;
    int start = 0;
    for (start = 0 ; start <= end ; start++)
    {
        revstr[start] = str[end - start];
    }
    revstr[end+1] = '\0';
    return revstr;
}