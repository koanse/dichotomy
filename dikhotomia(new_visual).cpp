#include <windows.h>

#define IDR_MENU1                       101
#define IDD_ENTER_FORMULA               102
#define IDD_OPTIMIZATION_DIALOG         103
#define IDD_CALCULATOR_DIALOG           104
#define IDD_ERROR_DIALOG                105
#define IDC_EDIT_FUNCTION               1001
#define IDC_OK_FUNC                     1002
#define IDC_EDIT_A                      1003
#define IDC_EDIT_B                      1004
#define IDC_EDIT_EPSILON                1005
#define IDC_CANCEL_FUNC                 1006
#define IDC_EDIT_X_MIN                  1006
#define IDC_EDIT_F_X_MIN                1007
#define IDC_START_OPTIMIZATION          1008
#define IDC_CANCEL_OPTIMIZATION         1009
#define IDC_EDIT_ARGUMENT               1009
#define IDC_EDIT_FUNCTION_VALUE         1010
#define IDC_CALCULATE                   1011
#define IDC_HELP_OPT                    1011
#define IDC_CANCEL_CALCULATE            1012
#define IDC_EXIT_ERROR                  1012
#define IDC_HELP_FUNC                   1013
#define IDC_HELP_CALC                   1016
#define IDC_EDIT1                       1018
#define IDC_ERROR_DESCRIPTION           1019
#define ID_EXIT_FROM_PROGRAM            40007
#define ID_CALCULATOR                   40008
#define ID_OPTIMIZATION                 40009
#define ID_FUNCTION                     40010
#define ID_CONTENTS                     40011

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK DlgProcEnterFormula(HWND hdlg, UINT message,
								  WPARAM wParam, LPARAM lParam);

BOOL CALLBACK DlgProcOptimization(HWND hdlg, UINT messg,
								  WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcCalculation(HWND hdlg, UINT messg,
                                 WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcErrorDlg(HWND hdlg, UINT messg,
                              WPARAM wParam, LPARAM lParam);
HWND hActiveWindow;
HINSTANCE hInstance;

char szProgName[] = "ProgName";
char formula[500];
char *translated_formula = NULL;
char error_type[1000];

/////////////////////////////////////////////////////////////

#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
double epsilon = 0.000001;
const char digits[] = "0123456789.eEdD";
const char operators_and_priors[] = "-+m/*^sScCl";
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
	tree_element *left, *right;
} *root;
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
void delete_tree(tree_element *cur);
int _matherr(_exception *exc);

///////////////////////////////////////////////////////////////
int WINAPI WinMain 	(HINSTANCE hInst,HINSTANCE hPreInst,
							LPSTR lpszCmdLine,int nCmdShow)
{
	HWND hWnd;
	HMENU hMenu;
	MSG lpMsg;
	WNDCLASS wcApp;

	hInstance = hInst;
	strcpy(formula, "[формула]");

	wcApp.lpszClassName	=	szProgName;
	wcApp.hInstance		=	hInst;
	wcApp.lpfnWndProc	=	WndProc;
	wcApp.hCursor		=	LoadCursor(NULL,IDC_ARROW);
	wcApp.hIcon			=	0;
	wcApp.lpszMenuName	=	"Menu1";
	wcApp.hbrBackground	=	(HBRUSH) GetStockObject(WHITE_BRUSH);
	wcApp.style			=	CS_HREDRAW | CS_VREDRAW;
	wcApp.cbClsExtra	=	0;
	wcApp.cbWndExtra	=	0;
	if(!RegisterClass(&wcApp))
		return 0;

	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));


	hWnd = CreateWindow	(szProgName, "Оптимизация функций",
						WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
						CW_USEDEFAULT, 300,
						150, (HWND)NULL,(HMENU)hMenu,
						(HINSTANCE)hInst, (LPSTR)NULL);



	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while(GetMessage(&lpMsg,0,0,0))
	{
   		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	return lpMsg.wParam;
}

LRESULT CALLBACK WndProc	(HWND hWnd,UINT messg,
							WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
		
	hActiveWindow = hWnd;
	
	switch(messg) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd,&ps);
		
         //MoveToEx(hdc,0,0,NULL);
         //LineTo(hdc,639,429);
         //MoveToEx(hdc,300,0,NULL);
         //LineTo(hdc,50,300);

		//TextOut(hdc, 150, 10, "F(x) =", 6);         
		//TextOut(hdc, 150, 30, formula, strlen(formula));

		ValidateRect(hWnd, NULL);
		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_EXIT_FROM_PROGRAM:
			PostQuitMessage(0);
			return 0;

		case ID_CONTENTS:
			strcpy(error_type,
					"ПРОГРАММНЫЙ ПРОДУКТ ДЛЯ ОПТИМИЗАЦИИ ФУНКЦИИ "
					"ОДНОЙ ПЕРЕМЕННОЙ С ВОЗМОЖНОСТЬЮ ВЫЧИСЛЕНИЯ "
					"ЗНАЧЕНИЯ МАТЕМАТИЧЕСКОГО ВЫРАЖЕНИЯ, "
					"ЗАДАВАЕМОГО ПОЛЬЗОВАТЕЛЕМ.\n\n\n"
					"     Для входных данных и в вычислениях "
					"используется вещественный формат данных "
					"со следующим диапазоном значений:\n"
					"1.7E +/- 308 (15 значащих цифр).\n\n"
					"     Результаты вычислений выдаются с "
					"десятью знаками после запятой.");
			DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
			break;

		case ID_FUNCTION:
			DialogBox(hInstance,
				MAKEINTRESOURCE(IDD_ENTER_FORMULA),
				hWnd, (DLGPROC) DlgProcEnterFormula);
			break;
		case ID_OPTIMIZATION:
			DialogBox(hInstance,
				MAKEINTRESOURCE(IDD_OPTIMIZATION_DIALOG),
				hWnd, (DLGPROC) DlgProcOptimization);
			break;

		case ID_CALCULATOR:
			DialogBox(hInstance,
				MAKEINTRESOURCE(IDD_CALCULATOR_DIALOG),
				hWnd, (DLGPROC) DlgProcCalculation);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,messg,wParam,lParam);
		break;
	}
	return 0;
}

BOOL CALLBACK DlgProcEnterFormula(HWND hdlg, UINT messg,
								  WPARAM wParam, LPARAM lParam)
{
	hActiveWindow = hdlg;
	
	switch(messg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hdlg, IDC_EDIT_FUNCTION, formula);
		return false;
	case WM_COMMAND:
		switch(wParam)
		{
        case IDC_CANCEL_FUNC:
			EndDialog(hdlg, FALSE);
			return false;
		case IDC_HELP_FUNC:
			strcpy(error_type,
					"     Длина выражения не должна превышать "
					"500 символов, в противном случае лишние "
					"символы отбрасываются.\n"
					"     Используемые операторы: "
					"- (унарный и бинарный), "
					"+, /, *, ^ (возведение в степень), "
					"s (синус), S (гиперболический синус), "
					"c (косинус), C (гиперболический косинус), "
					"l (натуральный логарифм). Приоритеты "
					"операторов - стандартные.\n"
					"     Для обозначения "
					"аргумента используйте символ x или X.\n"
					"     Количество скобок, используемых в "
					"выражении, ограничено лишь размером "
					"самого выражения.\n"
					"     В случае отсутствия парности "
					"открывающих и закрывающих скобок "
					"интерпретатор будет подразумевать их "
					"наличие, соответственно, в начале и конце "
					"выражения.\n"
					"     Для записи выражения можно "
					"использовать как обычную математическую, "
					"так и инфиксную запись. Допускается, но "
					"настоятельно не рекомендуется их "
					"комбинирование.\n"
					"     Формат записи констант стандартен:\n"
					"[whitespace] [sign] [digits] [.digits] "
					"[ {d | D | e | E }[sign]digits]");
			DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
						hActiveWindow, (DLGPROC) DlgProcErrorDlg);
			break;
				

		case IDC_OK_FUNC:
			char *tmpTrForm;
			tree_element *tmpRoot;
			GetDlgItemText(hdlg, IDC_EDIT_FUNCTION,
				formula, 499);
			
			tmpTrForm = translate_formula(formula);
			if(!tmpTrForm)
			{		
				strcpy(error_type,
					"Синтаксическая ошибка: один или "
					"несколько символов в формуле не "
					"подлежат интерпретации. Проверьте "
					"правильность ввода формулы.");
				DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
				delete tmpTrForm;
				break;
			}

			tmpRoot = make_translation_tree(tmpTrForm);
			if(!tmpRoot)
			{
				strcpy(error_type,
					"Синтаксическая ошибка: неправильное "
					"использование операторов. Проверьте "
					"правильность ввода формулы.");
				DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
				
				delete tmpTrForm;
				break;
			}
			
			delete translated_formula;
			delete_tree(root);
			
			translated_formula = tmpTrForm;
			root = tmpRoot;

			EndDialog(hdlg, FALSE);
			return false;
		}
		break;
	case WM_CLOSE:
		EndDialog(hdlg,FALSE);
		return false;
	}
	return false;
}

BOOL CALLBACK DlgProcOptimization(HWND hdlg, UINT messg,
								  WPARAM wParam, LPARAM lParam)
{
	hActiveWindow = hdlg;
	
	char a[500], b[500], eps[500], tmp[500];
	double x_min;	

	switch(messg)
	{
	case WM_INITDIALOG:
		sprintf(tmp, "%lf", epsilon);
		SetDlgItemText(hdlg, IDC_EDIT_EPSILON, tmp);

		return false;
	case WM_COMMAND:
		switch(wParam)
		{
        case IDC_CANCEL_OPTIMIZATION:
			EndDialog(hdlg,FALSE);
			return false;
		case IDC_HELP_OPT:
			strcpy(error_type,
					"     Данный программный продукт выполняет "
					"оптимизацию (т.е. нахождение минимума) "
					"функции одной переменной методом дихотомии "
					"на отрезке [a,b] c точностью, равной "
					"значению epsilon.\n"
					"     Задайте отрезок [a,b] так, чтобы на "
					"нем функция имела единственный экстремум - "
					"минимум.\n"
					"     Введите нужное значение epsilon, или "
					"используйте заданное по умолчанию.\n"
					"     В случае появления сообщения об "
					"ошибке результат оптимизации может быть "
					"неточным; для получения правильного "
					"результата необходимо отсутствие ошибок.");
			DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
			break;
		case IDC_START_OPTIMIZATION:
			if(!translated_formula)
			{		
				strcpy(error_type,
					"Перед выполнением оптимизации следует "
					"задать оптимизируемую функцию. ");
				DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
				break;
			}

			GetDlgItemText(hdlg, IDC_EDIT_A, a, 19);
			GetDlgItemText(hdlg, IDC_EDIT_B, b, 19);
			GetDlgItemText(hdlg, IDC_EDIT_EPSILON, eps, 19);
			
			epsilon = atof(eps);
			
			if(atof(a) > atof(b))
			{
				SetDlgItemText(hdlg, IDC_EDIT_A, b);
				SetDlgItemText(hdlg, IDC_EDIT_B, a);
				x_min = minimization(atof(b), atof(a));

			}
			else x_min = minimization(atof(a), atof(b));

			sprintf(tmp, "%.10lf", x_min);
			SetDlgItemText(hdlg, IDC_EDIT_X_MIN, tmp);

			sprintf(tmp, "%.10lf", F(x_min));
			SetDlgItemText(hdlg, IDC_EDIT_F_X_MIN, tmp);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hdlg,FALSE);
		return false;
	}
	return false;
}



BOOL CALLBACK DlgProcCalculation(HWND hdlg, UINT messg,
                                 WPARAM wParam, LPARAM lParam)
{
	hActiveWindow = hdlg;
		
	switch(messg)
	{
	case WM_INITDIALOG:
		return false;
	case WM_COMMAND:
		switch(wParam)
		{
        case IDC_CANCEL_CALCULATE:
			EndDialog(hdlg,FALSE);
			return false;
		case IDC_HELP_CALC:
			strcpy(error_type,
				"     Введите значение аргумента и нажмите "
				"кнопку Вычислить.\n"
				"     Встроенный в данный программный продукт "
				"интерпретатор математических выражений вычислит "
				"значение введенной функции при заданном "
				"аргументе.\n"
				"     В случае, если аргумент не принадлежит "
				"области определения функции (или в других "
				"особых ситуациях, например, переполнение, "
				"потеря значимости и т.д.) на экран будет "
				"выведено соответствующее сообщение.");
			DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
			break;

		case IDC_CALCULATE:
			char str[500];
			double x;

			if(!translated_formula)
			{		
				strcpy(error_type,
					"Перед выполнением вычислений следует "
					"задать вычисляемую функцию. ");
				DialogBox(hInstance,
					MAKEINTRESOURCE(IDD_ERROR_DIALOG),
					hActiveWindow, (DLGPROC) DlgProcErrorDlg);
				break;
			}

			GetDlgItemText(hdlg, IDC_EDIT_ARGUMENT, str, 19);
			x = atof(str);
			sprintf(str, "%.10lf", F(x));
			SetDlgItemText(hdlg, IDC_EDIT_FUNCTION_VALUE, str);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hdlg,FALSE);
		return false;
	}
	return false;
}



BOOL CALLBACK DlgProcErrorDlg(HWND hdlg, UINT messg,
                              WPARAM wParam, LPARAM lParam)
{
	switch(messg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hdlg, IDC_ERROR_DESCRIPTION, error_type);
		return false;
	case WM_COMMAND:
		switch(wParam)
		{
        case IDC_EXIT_ERROR:
			EndDialog(hdlg, FALSE);
			return false;
		}
	case WM_CLOSE:
		EndDialog(hdlg,FALSE);
		return false;
	}
	return false;
}

//////////////////////////////////////////////////////////

double minimization(double a, double b)
{
	double L, x1, x2, xm, Fxm, Fx1, Fx2;
	
	xm = (a + b)/2;
	Fxm = F(xm);
	L = b - a;
	L *= 2.0l;

	while(L > epsilon)
	{
		L /= 2.0l;

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

    k = 0;
	for(i = 0; form_start[i]; i++)
	{
		for(j = 0; digits[j]; j++)
			if(form_start[i] == digits[j]) break;
		if(digits[j])
			{ form[k++] = form_start[i]; continue; }

		if(form_start[i] == ' ')
			{ form[k++] = ' '; continue; }

		if(form_start[i] == '-')
		{
			if(i == 0) { form[k++] = 'm'; continue; }
			else
			{
                for(j = i - 1;j >= 0 &&
					form_start[j] == ' '; j--);
				if(j < 0 || form_start[j] == '(')
					{ form[k++] = 'm'; continue; }
			}
		}
		
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

		return NULL;
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

		if(is_a_space_necessary)
			{ result[k++] = ' '; i--; continue; }
	
	////////////////////////////////////////////

		if(form[i] == 'x')
		{
            result[k++] = 'x';
			result[k++] = ' ';
			continue;
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
			continue;
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
	delete form;

	return result;
}

tree_element* make_translation_tree(char *form)
{
	// построение дерева для вычисления выражения
	int i, j, k;
	element_of_stack_for_tree *tmp1, *end;
	tree_element *tmp2;
	char tmpstr[500];
	
	end = NULL;
	for(i = 0; form[i]; i++)
	{
		if(form [i] == ' ') continue;

		for(j = 0; digits[j]; j++)
			if(form[i] == digits[j]) break;

		if(digits[j])
		{
			k = 0;
			while(digits[j])
			{
				tmpstr[k++] = form[i++];
				for(j = 0; digits[j]; j++)
					if(form[i] == digits[j]) break;
			}
			tmpstr[k] = '\0';
		
			tmp2 = new tree_element;
			tmp2->name = 'k';
			tmp2->left = tmp2->right = NULL;
			tmp2->value = atof(tmpstr);

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
			}
			else
			{
                if(!end || !end->prev) return NULL;
				
				tmp2 = new tree_element;
				tmp2->right = end->elem;
				tmp2->left = end->prev->elem;
				tmp2->name = operators_and_priors[j];
				tmp2->value = 0.0;

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
			tmp2->left = tmp2->right = NULL;
			tmp2->value = 0.0f;

			tmp1 = new element_of_stack_for_tree;
			tmp1->elem = tmp2;
			tmp1->prev = end;
			end = tmp1;
			continue;
		}
	}

	if(!end || end->prev) return NULL;
	
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
    return 0;
}

int _matherr(_exception *exc)
{
	exc->retval = 0;

	switch(exc->type)
	{
	case DOMAIN:
		strcpy(error_type,
			"Значение аргумента не принадлежит области "
			"определения функции. Переопределите функцию "
			"или измените значение аргумента." );
		break;
	case SING:
		strcpy(error_type,
			"Ошибка из-за особенностей аргумента. "
			"Проверьте область определения функции и "
			"значение аргумента.");
		break;
	case OVERFLOW:
		strcpy(error_type,
			"Переполнение разрядной сетки. Переопределите "
			"функцию или уменьшите значение аргумента.");
		break;
	case UNDERFLOW:
		strcpy(error_type,
			"Число слишком мало для представления. "
			"Переопределите функцию или увеличьте значение "
			"аргумента.");
		break;
	case TLOSS:
		strcpy(error_type,
			"Полная потеря значимости. Переопределите "
			"функцию или измените значение аргумента.");
		break;
	case PLOSS:
		strcpy(error_type,
			"Частичная потеря значимости. Переопределите "
			"функцию или измените значение аргумента.");
		break;
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ERROR_DIALOG),
			hActiveWindow, (DLGPROC) DlgProcErrorDlg);
	return 1;
}				


void delete_tree(tree_element *cur)
{
	if(!cur) return;
	if(cur->left) delete_tree(cur->left);
	if(cur->right) delete_tree(cur->right);

	delete cur;
	return;
}