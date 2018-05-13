#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
const double epsilon = 0.000001f;
const char digits[] = "0123456789.eE";
const char operators_and_priors[] = "-+/*^sScClm";
const char unary_operators[] = "sScClm";
double argument_x;

struct operators
{
	char name;
	int prior;
	operators *prev;
};

struct tree_element
{
	char name;
	double value;
	tree_element *parent, *left, *right;
} extern *root;
struct element_of_stack_for_tree
{
	tree_element *elem;
	element_of_stack_for_tree *prev;
};

double minimization(double a, double b);
double F(double x);
char* translate_formula(char *form);
tree_element* make_translation_tree(char *form);
double process_function(tree_element *cur);
int _matherr(_exception *exc);
double minimization(double a, double b)
{
	double L, x1, x2, xm, Fxm, Fx1, Fx2;
	
	xm = (a + b)/2;
	Fxm = F(xm);
	L = b - a;
	L *= 2.0f;

	while(L > epsilon)
	{
		L /= 2.0f;

		x1 = a + L/4;
		x2 = b - L/4;

		Fx1 = F(x1);
		Fx2 = F(x2);

		if(Fx1 < Fxm)	{ b = xm; xm = x1; Fxm = Fx1; }
		else if(Fx2 < Fxm) { a = xm; xm = x2; Fxm = Fx2; }
		else { a = x1; b = x2; }
	}
	
	return xm;
}

double F(double x)
{
	argument_x = x;
	return process_function(root);
}
char* translate_formula(char *form_start)
{
    // преобразование в постфиксную запись
	char *result = new char [5 * strlen(form_start)];
	char *form = new char [5 * strlen(form_start)];
	int i, j, k, cur_prior, bonus_to_prior;
	bool is_a_space_necessary;
	operators *end, *tmp;
	end = NULL;

    // предварительная обработка
	k = 0;
	for(i = 0; form_start[i]; i++)
	{
		for(j = 0; digits[j]; j++)
			if(form_start[i] == digits[j]) break;
		if(digits[j])
		{
			form[k++] = form_start[i];
			if(form_start[i+1] == 'x' ||
				form_start[i+1] == 'X') form[k++] = '*';
			continue;
		}

		if(form_start[i] == '-' &&
			(i == 0 || form_start[i-1] == '('))
			{ form[k++] = 'm'; continue; }
		
		if(form_start[i] == 'x' || form_start[i] == 'X')
			{ form[k++] = 'x'; continue; }

		if(form_start[i] == '(')
			{ form[k++] = '('; continue; }

		if(form_start[i] == ')')
			{ form[k++] = ')'; continue; }
		
		for(j = 0; operators_and_priors[j]; j++)
			if(form_start[i] == operators_and_priors[j]) break;
		if(operators_and_priors[j])
			{ form[k++] = form_start[i]; continue; }
	}
	form[k] = '\0';
	
	////////////////////////////////////////////
	k = 0;
	bonus_to_prior = 0;
	for(i = 0; form[i]; i++)
	{
		is_a_space_necessary = false;
		do for(j = 0; digits[j]; j++)
				if(form[i] == digits[j])
				{
					result[k++] = form[i++];
					is_a_space_necessary = true;
					break;
				}
		while(digits[j] && form[i]);

		if(is_a_space_necessary) result[k++] = ' ';
	
	////////////////////////////////////////////

		if(form[i] == 'x')
		{
            result[k++] = 'x';
			result[k++] = ' ';
		}

	////////////////////////////////////////////

		for(j = 0; operators_and_priors[j]; j++)
			if(form[i] == operators_and_priors[j]) break;

		if(operators_and_priors[j])
		{
			cur_prior = j;
			while(end)
				if(end->prior >= cur_prior + bonus_to_prior)
				{
					result[k++] = end->name;
					result[k++] = ' ';
					tmp = end;
					end = end->prev;
					delete tmp; 
				}
				else break;
			tmp = new operators;
			tmp->name = form[i];
			tmp->prior = cur_prior + bonus_to_prior;
			tmp->prev = end;
            end = tmp;
		}
	////////////////////////////////////////////

		if(form[i] == '(') bonus_to_prior += 50;
		if(form[i] == ')') bonus_to_prior -= 50;
	}
	
	while(end)
	{
		result[k++] = end->name;
		result[k++] = ' ';
		tmp = end;
		end = end->prev;
		delete tmp;
	}
	result[k] = '\0';
	delete end;

	return result;
}		





tree_element* make_translation_tree(char *form)
{
	// построение дерева для вычисления выражения
	int i, j, k;
	element_of_stack_for_tree *tmp1, *end;
	tree_element *tmp2;
	char tmpstr[30];
	
	end = NULL;
	for(i = 0; form[i]; i++)
	{
		if(form [i] == ' ') i++;

		for(j = 0; digits[j]; j++)
			if(form[i] == digits[j]) break;

		if(digits[j])
		{
			k = 0;
			tmpstr[k++] = form[i++];
			while(form[i] && digits[j])
				for(j = 0; digits[j]; j++)
					if(form[i] == digits[j])
					{
						tmpstr[k++] = form[i++];
						break;
					}
			tmpstr[k] = '\0';
			
			tmp2 = new tree_element;
			tmp2->name = 'k';
			tmp2->left = tmp2->right = tmp2->parent = NULL;
			tmp2->value = (float) atof(tmpstr);

			tmp1 = new element_of_stack_for_tree;
			tmp1->elem = tmp2;
			tmp1->prev = end;
			end = tmp1;

			continue;
		}

		////////////////////////////////////////////////////

		for(j = 0; operators_and_priors[j]; j++)
			if(form[i] == operators_and_priors[j]) break;
		
		if(operators_and_priors[j])
		{
			for(k = 0; unary_operators[k]; k++)
				if(form[i] == unary_operators[k]) break;

			if(unary_operators[k])
			{
                tmp2 = new tree_element;
				tmp2->right = end->elem;
				tmp2->left = NULL;
				tmp2->name = unary_operators[k];
				tmp2->value = 0.0;
	
				tmp2->right->parent = tmp2;
			}
			else
			{
                if(!end || !end->prev) return NULL;
				
				tmp2 = new tree_element;
				tmp2->right = end->elem;
				tmp2->left = end->prev->elem;
				tmp2->name = operators_and_priors[j];
				tmp2->value = 0.0;

				tmp2->left->parent = tmp2->right->parent = tmp2;

				tmp1 = end;
				end = end->prev;
				delete tmp1;
			}
			
			end->elem = tmp2;
			continue;
		}

		////////////////////////////////////////////////////
		
		if(form[i] == 'x')
		{
			tmp2 = new tree_element;
			tmp2->name = 'x';
			tmp2->left = tmp2->right = tmp2->parent = NULL;
			tmp2->value = 0.0f;

			tmp1 = new element_of_stack_for_tree;
			tmp1->elem = tmp2;
			tmp1->prev = end;
			end = tmp1;
		}
	}

	if(end->prev) return NULL;
	
	tmp2 = end->elem;
	delete end;

	return tmp2;
}

double process_function(tree_element *cur)
{
	// рекурсивная функция, вычисляющая значение выражения
	// при помощи сгенерированного дерева
	double operand1, operand2;
		
	if(!cur) return 0;
	if(cur->left) operand1 = process_function(cur->left);
	if(cur->right) operand2 = process_function(cur->right);

	if(!cur->left && !cur->right)
	{
		if(cur->name == 'k') return cur->value;
		if(cur->name == 'x') return argument_x;
	}
	
	switch(cur->name)
	{
	case '-':
		return operand1 - operand2;
	case '+':
		return operand1 + operand2;
	case '/':
		return operand1 / operand2;
	case '*':
		return operand1 * operand2;
	case '^':
		return pow(operand1, operand2);
	case 's':
		return sin(operand2);
	case 'S':
		return sinh(operand2);
	case 'c':
		return cos(operand2);
	case 'C':
		return cosh(operand2);
	case 'l':
		return log(operand2);
	case 'm':
		return -operand2;
	}
    return 0.0;
}

int _matherr(_exception *exc)
{
	exc->retval = 0;

	switch(exc->type)
	{
	case DOMAIN:
		//printf("Аргумент не принадлежит области определения.");
		return 1;
	case SING:
		//printf("Ошибка из-за особенностей аргумента.");
		return 1;
	case OVERFLOW:
		//printf("Переполнение.");
		return 1;
	case UNDERFLOW:
		//printf("Число слишком мало для представления.");
		return 1;
	case TLOSS:
		//printf("Полная потеря значимости.");
		return 1;
	case PLOSS:
		//printf("Частичная потеря значимости.");
		return 1;
	}
	return 1;
}				
