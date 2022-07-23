/*
 *  A recursive-descent parser for a regular expression subset, (the empty
 *  language is not included).
 *
 *  Regular Expression grammar ('#' is the empty content):
 *  RE  ::= # | symbol | RE + RE | RE RE | RE * | ( RE ).
 *
 *  Left recursion removal:
 *  RE  ::= # | # RE' | symbol | symbol RE' | ( RE ) | ( RE ) RE'
 *  RE' ::= + RE | + RE RE' | RE | RE RE' | * | * RE'.
 */

#include "RE_parser.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CONTENT_LEN 16 // Max characters of the content of a node.
#define MAX_CHILDS 4 // Max number of childs for a variable in the parse tree.
#define INDENTATION 1 // Child indentation when the parse tree is printed.

/**** Parse tree functions and data structures. ****/
struct Node
{
  char content   [MAX_CONTENT_LEN];
  Node * childs [MAX_CHILDS];
};

Node * node_new (void)
{
  return malloc(sizeof(Node));
}

void node_init (Node * const p_node, const char * s)
{
  for (int i = 0; i < MAX_CHILDS; ++i)
    p_node->childs[i] = NULL;

  strcpy(p_node->content, s);
}

void node_add_child (Node * const p_node, Node * const p_child)
{
  for (int i = 0; i < MAX_CHILDS; ++i)
  {
    if (p_node->childs[i] == NULL)
    {
      p_node->childs[i] = p_child;
      break;
    }
  }
}

void node_free (Node * p_node)
{
  if (NULL != p_node)
  {
    for (int i = 0; i < MAX_CHILDS; ++i)
    {
      node_free(p_node->childs[i]);
      p_node->childs[i] = NULL;
    }
    free(p_node);
  }
}

// Backtraking: release the last n childs added by n failure branches.
void node_free_last_childs (Node * const p_node, int n)
{
  for (int i = MAX_CHILDS-1; i >= 0; --i)
  {
    if (p_node->childs[i] != NULL)
    {
      node_free(p_node->childs[i]);
      p_node->childs[i] = NULL;
      --n;
    }
    if (n == 0)
      return;
  }
}

void node_free_childs (Node * const p_node)
{
  for (int i = 0; i < MAX_CHILDS; ++i)
  {
    node_free(p_node->childs[i]);
    p_node->childs[i] = NULL;
  }
}

void tree_print (const Node * const p_node, int indent)
{
  if (NULL != p_node)
  {
    for (int i = 0; i < indent; ++i)
    {
      printf("-");
    }
    printf("%s\n", p_node->content);
    for (int i = 0; i < MAX_CHILDS; ++i)
    {
      tree_print(p_node->childs[i], indent + INDENTATION);
    }
  }
}

/**** Terminals ****/

bool epsilon (const char *reg_expr,
              const int * const p_idx_in,
              int * const p_idx_out,
              Node * const p_node)
{
  const int i = *p_idx_in;
  if (reg_expr[i] == '#')               // Use '#' as epsilon.
  {
    *p_idx_out = i + 1;                 // Index of the next lexeme.
    Node * p_node_eps = node_new();
    node_init(p_node_eps, "#");
    node_add_child(p_node, p_node_eps); // Add the node to the parse tree.
    return true;
  }

  return false;
}

bool symbol (const char *reg_expr,
             const int * const p_idx_in,
             int * const p_idx_out,
             Node * const p_node)
{
  const int i = *p_idx_in;

  if (   (reg_expr[i] == '_')
      || (48 <= reg_expr[i] && reg_expr[i] <= 57)   // Digits.
      || (65 <= reg_expr[i] && reg_expr[i] <= 90)   // Caps letters.
      || (97 <= reg_expr[i] && reg_expr[i] <= 122)) // Letters.
  {
    *p_idx_out = i + 1;
    Node * p_node_symbol = node_new();
    char s[2];
    s[0] = reg_expr[i];
    s[1] = '\0';
    node_init(p_node_symbol, s);
    node_add_child(p_node, p_node_symbol);
    return true;
  }

  return false;
}

bool lpar (const char *reg_expr,
           const int * const p_idx_in,
           int * const p_idx_out,
           Node * const p_node)
{
  const int i = *p_idx_in;

  if (40 == reg_expr[i])
  {
    *p_idx_out = i + 1;
    Node * p_node_lpar = node_new();
    node_init(p_node_lpar, "(");
    node_add_child(p_node, p_node_lpar);
    return true;
  }

  return false;
}

bool rpar (const char *reg_expr,
           const int * const p_idx_in,
           int * const p_idx_out,
           Node * const p_node)
{
  const int i = *p_idx_in;

  if (41 == reg_expr[i])
  {
    *p_idx_out = i + 1;
    Node * p_node_rpar = node_new();
    node_init(p_node_rpar, ")");
    node_add_child(p_node, p_node_rpar);
    return true;
  }

  return false;
}

bool star (const char *reg_expr,
           const int * const p_idx_in,
           int * const p_idx_out,
           Node * const p_node)
{
  const int i = *p_idx_in;

  if (42 == reg_expr[i])
  {
    *p_idx_out = i + 1;
    Node * p_node_star = node_new();
    node_init(p_node_star, "*");
    node_add_child(p_node, p_node_star);
    return true;
  }

  return false;
}

bool plus (const char *reg_expr,
           const int * const p_idx_in,
           int * const p_idx_out,
           Node * const p_node)
{
  const int i = *p_idx_in;

  if (43 == reg_expr[i])
  {
    *p_idx_out = i + 1;
    Node * p_node_plus = node_new();
    node_init(p_node_plus, "+");
    node_add_child(p_node, p_node_plus);
    return true;
  }

  return false;
}

/**** Variables. ****/

// RE' ::= + RE | + RE RE' | RE | RE RE' | * | * RE'.
bool RE_prime (const char *reg_expr,
               const int * const p_idx_in,
               int * const p_idx_out,
               Node * const p_node)
{
  int idx_tmp1;
  int idx_tmp2;

  Node * p_RE_prime = node_new();
  node_init(p_RE_prime, "RE'");
  node_add_child(p_node, p_RE_prime);

  // RE' -> + RE RE'
  if (plus(reg_expr, p_idx_in, &idx_tmp1, p_RE_prime))
  {
    if (RE(reg_expr, &idx_tmp1, &idx_tmp2, p_RE_prime))
    {
      if(RE_prime(reg_expr, &idx_tmp2, p_idx_out, p_RE_prime))
        return true;
      else
        node_free_last_childs(p_RE_prime, 2);
    }
    else
      node_free_last_childs(p_RE_prime, 1);
  }

  // RE' -> + RE.
  if (plus(reg_expr, p_idx_in, &idx_tmp1, p_RE_prime))
  {
    if (RE(reg_expr, &idx_tmp1, p_idx_out, p_RE_prime))
      return true;
    else
      node_free_last_childs(p_RE_prime, 1);
  }

  // RE' -> * RE'.
  if (star(reg_expr, p_idx_in, &idx_tmp1, p_RE_prime))
  {
    if (RE_prime(reg_expr, &idx_tmp1, p_idx_out, p_RE_prime))
      return true;
    else
      node_free_last_childs(p_RE_prime, 1);
  }

  // RE' -> RE RE'.
  if (RE(reg_expr, p_idx_in, &idx_tmp1, p_RE_prime))
  {
    if (RE_prime(reg_expr, &idx_tmp1, p_idx_out, p_RE_prime))
      return true;
    else
      node_free_last_childs(p_RE_prime, 1);
  }

  // RE' -> RE.
  if (RE(reg_expr, p_idx_in, p_idx_out, p_RE_prime))
    return true;

  // RE' -> *.
  if (star(reg_expr, p_idx_in, p_idx_out, p_RE_prime))
    return true;

  node_free_last_childs(p_node, 1); // Free `p_RE_prime' and all of its childs.
  return false;
}

// RE ::= # | # RE' | symbol | symbol RE' | ( RE ) | ( RE ) RE'.
bool RE (const char *reg_expr,
         const int * const p_idx_in,
         int * const p_idx_out,
         Node * const p_node)
{
  int idx_tmp1, idx_tmp2, idx_tmp3;

  Node *p_RE = node_new();
  node_init(p_RE, "RE");
  node_add_child(p_node, p_RE);

  // RE -> # RE'.
  if (epsilon(reg_expr, p_idx_in, &idx_tmp1, p_RE))
  {
    if (RE_prime(reg_expr, &idx_tmp1, p_idx_out, p_RE))
      return true;
    else
      node_free_last_childs(p_RE, 1);
  }

  // RE -> symbol RE'.
  if (symbol(reg_expr, p_idx_in, &idx_tmp1, p_RE))
  {
    if (RE_prime(reg_expr, &idx_tmp1, p_idx_out, p_RE))
      return true;
    else
      node_free_last_childs(p_RE, 1);
  }

  // RE -> ( RE ) RE'.
  if (lpar(reg_expr, p_idx_in, &idx_tmp1, p_RE))
  {
    if (RE(reg_expr, &idx_tmp1, &idx_tmp2, p_RE))
    {
      if (rpar(reg_expr, &idx_tmp2, &idx_tmp3, p_RE))
      {
        if (RE_prime(reg_expr, &idx_tmp3, p_idx_out, p_RE))
          return true;
        else
          node_free_last_childs(p_RE, 3);
      }
      else
        node_free_last_childs(p_RE, 2);
    }
    else
      node_free_last_childs(p_RE, 1);
  }

  // RE -> ( RE ).
  if (lpar(reg_expr, p_idx_in, &idx_tmp1, p_RE))
  {
    if (RE(reg_expr, &idx_tmp1, &idx_tmp2, p_RE))
    {
      if (rpar(reg_expr, &idx_tmp2, p_idx_out, p_RE))
        return true;
      else
        node_free_last_childs(p_RE, 2);
    }
    else
      node_free_last_childs(p_RE, 1);
  }

  // RE -> #.
  if (epsilon(reg_expr, p_idx_in, p_idx_out, p_RE))
    return true;

  // RE -> symbol.
  if (symbol(reg_expr, p_idx_in, p_idx_out, p_RE))
    return true;

  node_free_last_childs(p_node, 1); // Free `p_RE' and all of its childs.
  return false;
}

bool parse (const char *reg_expr, Node *p_node)
{
  int start_index = 0;
  int end_index = 0;

  node_init(p_node, "Root");
  bool parse_result = RE(reg_expr, &start_index , &end_index, p_node);

  return (parse_result && reg_expr[end_index] == '\0');
}

int main (int argc, char **argv)
{
  // Input checks.
  if (argc != 2)
  {
    printf("Wrong number of command-line arguments: ");
    printf("%d arguments found, %d expected\n", argc -1, 1);
    return 1;
  }

  Node tree;

  if (parse(argv[1], &tree))
  {
    tree_print(tree.childs[0], 0); // Do not print the "root" node.
  }
  else
  {
    printf("Syntax error\n");
  }

  node_free_childs(&tree);

  return 0;
}
