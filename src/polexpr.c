#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR   1024
#define MAX_STACK 1024

#define TYPE_LFT_PAR 4
#define TYPE_BI_OPR  3
#define TYPE_UN_OPR  2
#define TYPE_NUM     1
#define TYPE_VAR     0

//
typedef unsigned char byte;

//
typedef struct node_s {
  byte type;
  unsigned val;
  unsigned char str[MAX_STR];
  struct node_s *left;
  struct node_s *right; } node_t;

//
typedef struct node_stack_s { int sp; node_t *stack[MAX_STACK]; } node_stack_t;

//
int init_stack(node_stack_t *s)
{ s->sp = -1; }

//
node_t *new_node(byte type, unsigned val, unsigned char *str)
{
  node_t *n = malloc(sizeof(node_t));

  if (n)
    {
      n->type = type;
      n->val  = val;
      n->left = n->right = NULL;
      
      if (str)
	strncpy(n->str, str, strlen(str));
      else
	str = NULL;
    }
  
  return n;
}

//
int push(node_stack_t *s, node_t *n)
{
  if (s->sp < MAX_STACK)
    {
      s->stack[++s->sp] = n;

      return 1;
    }
  else
    return 0;;
}

//
int pop(node_stack_t *s, node_t **n)
{
  if (s->sp > -1)
    {
      *n = s->stack[s->sp--];

      return 1;
    }
  else
    {
      *n = NULL;
      
      return 0;
    }
}

//
int top(node_stack_t *s, node_t **n)
{
  if (s->sp > -1)
    {
      *n = s->stack[s->sp];

      return 1;
    }
  else
    {
      *n = NULL;
      
      return 0;
    }
}

//
int is_opr(unsigned char c)
{
  switch (c)
    {

    case '^':
    case '*':
    case '/':
    case '%':
    case '+':
    case '-':
      return 1;
      
    default:
      return 0;
    }
}

//
int get_op_priority(unsigned char c)
{
  switch (c)
    {
    case '^':
      return 4;

    case '*':
    case '/':
    case '%':
      return 3;

    case '+':
    case '-':
      return 2;

    case '(':
      return 1;

    default:
      return 0;
    }
}

//
int get_op_type(unsigned char c)
{
  switch (c)
    {
    case '^':
    case '*':
    case '/':
    case '%':
    case '+':
    case '-':
      return TYPE_BI_OPR;

    default:
      return TYPE_UN_OPR;
    }
}

//
int is_alpha(unsigned char c)
{ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

//
int is_digit(unsigned char c)
{ return (c >= '0' && c <= '9'); }

//
int to_digit(unsigned char c)
{ return c - '0'; }

//
int get_number(unsigned char *buffer, int *val)
{
  int tmp = 0;
  unsigned i = 0;
  
  while (buffer[i] && is_digit(buffer[i]))
    {
      tmp *= 10;
      tmp += to_digit(buffer[i++]);
    }

  *val = tmp;
  
  return i;
}

//
int get_string(unsigned char *buffer, unsigned char *out)
{
  unsigned i = 0, j = 0;
  
  while (buffer[i] && is_alpha(buffer[i]))
    out[j++] = buffer[i++];
  
  out[j] = 0;
  
  return j;
}

//
int walk(unsigned char *buffer)
{
  unsigned steps = 0;
  
  while (buffer[steps] && buffer[steps] == ' ') steps++;

  return steps;
}

//
node_t *eval(unsigned char *buffer)
{
  byte valid = 1;
  int tmp_num = 0;
  unsigned steps = walk(buffer);
  unsigned char tmp_out[MAX_STR];
  unsigned bufflen = strlen(buffer);
  node_stack_t *stL = malloc(sizeof(node_stack_t)), *stR = malloc(sizeof(node_stack_t));

  init_stack(stL);
  init_stack(stR);
  
  while (steps < bufflen && buffer[steps] && valid)
    {      
      //Functions and variables
      if (is_alpha(buffer[steps]))
	{
	  steps += get_string(buffer + steps, tmp_out);
	  
	  //Assume it's a variable for testing

	  node_t *n = new_node(TYPE_VAR, 0, tmp_out);

	  push(stR, n);

	  printf("%s ", tmp_out);
	  
	  steps += walk(buffer + steps);	  
	}
      else
	if (is_digit(buffer[steps]))
	  {
	    steps += get_number(buffer + steps, &tmp_num);
	    
	    node_t *n = new_node(TYPE_NUM, tmp_num, NULL);
	    
	    push(stR, n);

	    printf("%d ", tmp_num);
	    
	    steps += walk(buffer + steps);
	  }
	else
	  if (is_opr(buffer[steps]))
	    {
	      node_t *top_n;

	      top(stL, &top_n);
	      
	      //If top_opr < opr
	      if (top_n == NULL || (top_n && get_op_priority(buffer[steps]) > get_op_priority(top_n->val)))
		{
		  //TYPE_[BI | UN]_OPR, '+' | '-' | ..., str
		  node_t *n = new_node(get_op_type(buffer[steps]), buffer[steps], NULL); 
		  
		  push(stL, n);
		}
	      else //If top_opr >= opr
		{
		  node_t *op1 = NULL, *op2 = NULL, *opr = NULL;
		  
		  pop(stL, &opr);
		  pop(stR, &op2);
		  pop(stR, &op1);

		  opr->left  = op1;
		  opr->right = op2;

		  push(stR, opr);

		  node_t *n = new_node(get_op_type(buffer[steps]), buffer[steps], NULL);

		  push(stL, n);
		}
	      
	      printf("%c ", buffer[steps]);
	      
	      steps++;
	      steps += walk(buffer + steps);
	    }
	  else
	    if (buffer[steps] == '(')
	      {
		node_t *n = new_node(TYPE_LFT_PAR, buffer[steps], NULL);

		push(stL, n);

		printf("( ");
		
		steps++;
		steps += walk(buffer + steps);
	      }
	    else
	      if (buffer[steps] == ')')
		{
		  node_t *op1 = NULL, *op2 = NULL, *opr = NULL;
		  
		  while (pop(stL, &opr) && opr->type != TYPE_LFT_PAR)
		    {
		      pop(stR, &op2);
		      pop(stR, &op1);
		      
		      opr->left  = op1;
		      opr->right = op2;
		      
		      push(stR, opr);
		    }

		  printf(") ");
		  steps++;
		  steps += walk(buffer + steps);
		}
	      else
		valid = 0;
    }

  node_t *op1 = NULL, *op2 = NULL, *opr = NULL;
  
  while (pop(stL, &opr))
    {
      pop(stR, &op2);
      pop(stR, &op1);
      
      opr->left  = op1;
      opr->right = op2;
      
      push(stR, opr);
    }
  
  free(stL);
  free(stR);

  pop(stR, &opr);

  printf("\n");
  
  return (valid) ? opr : NULL;
}

//Avoid the last comma
void _print_postfix_(node_t *t, unsigned depth)
{
  if (t)
    {
      _print_postfix_(t->left, depth + 1);
      _print_postfix_(t->right, depth + 1);
      
      if (t->type == TYPE_VAR)
	printf("%s%s", t->str, (depth) ? ", " : " ");
      else
	if (t->type == TYPE_NUM)
	  printf("%d%s", t->val, (depth) ? ", " : " ");
	else
	  if (t->type == TYPE_BI_OPR || t->type == TYPE_UN_OPR)
	    printf("%c%s", t->val, (depth) ? ", " : " ");
    }
}

//
void _print_tree_(node_t *t, unsigned depth)
{
  if (t)
    {
      for (unsigned i = 0; i < depth; i++)
	printf("|");
      
      if (t->type == TYPE_VAR)
	printf("%s\n", t->str);
      else
	if (t->type == TYPE_NUM)
	  printf("%d\n", t->val);
	else
	  if (t->type == TYPE_BI_OPR || t->type == TYPE_UN_OPR)
	    printf("%c\n", t->val);
      
      _print_tree_(t->left, depth + 1);
      _print_tree_(t->right, depth + 1);
    }
}

//Obvious wrapper for postfix 
void print_postfix(node_t *n)
{ _print_postfix_(n, 0); }

//Obvious warpper tree
void print_tree(node_t *n)
{ _print_tree_(n, 0); }

//Free the tree memory !!!

//
int main(int argc, char **argv)
{
  if (argc < 2)
    return printf("OUPS! %s expression\n", argv[0]), -1;

  printf("Expression: ");
  node_t *n = eval(argv[1]);
  
  if (!n) return printf("ERROR unrecognized character\n"), -1;
  
  printf("\nPolish notation: ");
  print_postfix(n); printf("\n\n");

  printf("Expression tree: ");
  print_tree(n); printf("\n");
  
  return 0;
}
