/*
 *  A recursive-descent parser for regular expressions.
 *
 *  Regular Expression grammar ('#' is the empty string):
 *  RE  ::= # | symbol | RE + RE | RE RE | RE * | ( RE ).
 *
 *  Left recursion removal:
 *  RE  ::= # | # RE' | symbol | symbol RE' | ( RE ) | ( RE ) RE',
 *  RE' ::= + RE | + RE RE' | RE | RE RE' | * | * RE'.
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>

/**** Parse tree functions and data structures. ****/

typedef struct Node Node;

Node * node_new(void);

void node_init(Node * const p_node, const char *s);

void node_add_child(Node * const p_node, Node * const p_child);

void node_free(Node * p_node);

void node_free_last_child(Node * const p_node);

void node_free_children(Node * const p_node);

void node_print (const Node * const p_node, int indent);

void node_save (Node *p_node, FILE *fp, int indent);

/**** Terminals. ****/

bool epsilon(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool symbol(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool lpar(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool rpar(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool star(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool plus(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

/**** Variables. ****/

bool RE_prime(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool RE(
  const char *rexpr,
  const int *p_idx_int,
  int * const p_idx_out,
  Node * const p_node);

bool parse (
  const char *rexpr,
  Node * const p_node);
