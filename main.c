#include <stdio.h>
#include <assert.h>

#include <windows.h>
#include <windowsx.h>

enum {

  COLOR = 0xAACCBB

} ;

enum {

  USER_MAX = 32

} ;

enum {

  FLAG_NONE   = 0      ,
  FLAG_START  = 1 << 1

} ;

typedef struct {

  int uid  ;
  int flag ;

  struct {
    int x ;
    int y ;
  } pos ;

  struct {
    int      size ;
    COLORREF rgb  ;
  } pen;

} user_t ;

static HWND canvas ;

static const char * name = "mousemux.api.target" ;

static void * users[USER_MAX];

static void func_motion(WPARAM W, LPARAM L);
static void func_button(WPARAM W, LPARAM L);
static void func_v_wheel(WPARAM W, LPARAM L);
static void func_h_wheel(WPARAM W, LPARAM L);
static void func_key(WPARAM W, LPARAM L);

enum {

  MOUSEMUX_MS_BUTTON_MIN   = 0       ,
  MOUSEMUX_MS_BUTTON_NONE  = 0       ,

  MOUSEMUX_MS_BUTTON_1_DN  = 1 << 0  , /* left */
  MOUSEMUX_MS_BUTTON_1_UP  = 1 << 1  ,
  MOUSEMUX_MS_BUTTON_2_DN  = 1 << 2  , /* right */
  MOUSEMUX_MS_BUTTON_2_UP  = 1 << 3  ,
  MOUSEMUX_MS_BUTTON_3_DN  = 1 << 4  , /* middle */
  MOUSEMUX_MS_BUTTON_3_UP  = 1 << 5  ,
  MOUSEMUX_MS_BUTTON_4_DN  = 1 << 6  , /* X1 */
  MOUSEMUX_MS_BUTTON_4_UP  = 1 << 7  ,
  MOUSEMUX_MS_BUTTON_5_DN  = 1 << 8  , /* X2 */
  MOUSEMUX_MS_BUTTON_5_UP  = 1 << 9  ,

  MOUSEMUX_MS_BUTTON_MAX   = 1 << 10
};

static struct {

  const char * name ;

  void (*func)(WPARAM W, LPARAM L);

  int message ;

} list[] = {

  { "mousemux.api.none"    , NULL         , 0 },
  { "mousemux.api.motion"  , func_motion  , 0 },
  { "mousemux.api.button"  , func_button  , 0 },
  { "mousemux.api.v.wheel" , func_v_wheel , 0 },
  { "mousemux.api.h.wheel" , func_v_wheel , 0 },
  { "mousemux.api.key"     , func_key     , 0 },

  { NULL }

};

static void user_pen_draw(void * paint)
{
  user_t * this = paint ;

  if(1)
  {
    HDC dc = GetDC(canvas);

    if(NULL != dc)
    {
      HPEN pen ;

      POINT p ;

      p.x = this -> pos.x ;
      p.y = this -> pos.y ;

      if(0 == ScreenToClient(canvas, &p))
      {
      }

      if(FLAG_START == (FLAG_START & this -> flag))
      {
        if(0 == MoveToEx(dc, p.x, p.y, NULL))
        {
        }

        this -> flag &= ~FLAG_START ;
      }

      if(NULL != (pen = CreatePen(PS_SOLID, this -> pen.size, this -> pen.rgb)))
      {
        if(0 == LineTo(dc, p.x, p.y))
        {
        }

        if(0 == DeleteObject(SelectObject(dc, pen)))
        {
        }
      }

      if(0 == ReleaseDC(canvas, dc))
      {
      }
    }
  }
}

static void * get_user(int uid)
{
  assert(uid > 0);
  assert(uid < USER_MAX);

  return users[uid];
}

static void func_motion(WPARAM W, LPARAM L)
{
  int uid = LOWORD(W);
  int hit = HIWORD(W);

  user_t * this = get_user(uid);

  this -> pos.x = GET_X_LPARAM(L);
  this -> pos.y = GET_Y_LPARAM(L);

  user_pen_draw(this);

  printf("user %d motion %d,%d hittest:%d\n", uid, this -> pos.x, this -> pos.y, hit);
}

static void func_button(WPARAM W, LPARAM L)
{
  int uid = LOWORD(W);
  int val = HIWORD(W);

  user_t * this = get_user(uid);

  this -> pos.x = GET_X_LPARAM(L);
  this -> pos.y = GET_Y_LPARAM(L);

  /* left button changes pen size */

  if(MOUSEMUX_MS_BUTTON_1_DN == (MOUSEMUX_MS_BUTTON_1_DN & val))
  {
    this -> pen.size += 10;
  }
  else
  if(MOUSEMUX_MS_BUTTON_1_UP == (MOUSEMUX_MS_BUTTON_1_UP & val))
  {
    this -> pen.size -= 10;
  }

  /* right button changes color */

  if((MOUSEMUX_MS_BUTTON_2_DN == (MOUSEMUX_MS_BUTTON_2_DN & val)) ||
     (MOUSEMUX_MS_BUTTON_2_UP == (MOUSEMUX_MS_BUTTON_2_UP & val)))
  {
    const int R = rand() % 255 ;
    const int G = rand() % 255 ;
    const int B = rand() % 255 ;

    this -> pen.rgb = RGB(R, G, B);
  }

  printf("user %d button value %d at %d,%d\n", uid, val, this -> pos.x, this -> pos.y);
}

static void func_v_wheel(WPARAM W, LPARAM L)
{
  int uid = LOWORD(W);
  int val = HIWORD(W);

  user_t * this = get_user(uid);

  this -> pos.x = GET_X_LPARAM(L);
  this -> pos.y = GET_Y_LPARAM(L);

  printf("user %d vertical wheel value %d at %d,%d\n", uid, val, this -> pos.x, this -> pos.y);
}

static void func_h_wheel(WPARAM W, LPARAM L)
{
  int uid = LOWORD(W);
  int val = HIWORD(W);

  user_t * this = get_user(uid);

  this -> pos.x = GET_X_LPARAM(L);
  this -> pos.y = GET_Y_LPARAM(L);

  printf("user %d horizontal wheel value %d at %d,%d\n", uid, val, this -> pos.x, this -> pos.y);
}

static void func_key(WPARAM W, LPARAM L)
{
}

static void sdk_init(void)
{
  int i;

  for(i = 0; NULL != list[i].name; i++)
  {
    if(0 == (list[i].message = RegisterWindowMessage(list[i].name)))
    {
      assert(0);
    }

    printf("SDK code %s registered as %d\n", list[i].name, list[i].message);
  }
}

static LRESULT class_handle(UINT message, WPARAM W, LPARAM L)
{
  int i;

  for(i = 0; NULL != list[i].name; i++)
  {
    if(list[i].message == message)
    {
      list[i].func(W, L);
    }
  }

  return 0;
}

static LRESULT CALLBACK class_wndproc(HWND hwnd, UINT message, WPARAM W, LPARAM L)
{
  switch(message)
  {
    case WM_CREATE :
                     break ;

    case WM_DESTROY:
                     break ;

    default        : if(0 == class_handle(message, W, L))
                     {
                     }

                     break ;
  }

  return DefWindowProc(hwnd, message, W, L);
}

static int class_register(void)
{
  WNDCLASSEX wc = { 0 };

  wc.cbSize = sizeof wc;

  if(0 == GetClassInfoEx(NULL, "static", &wc))
  {
  }

  wc.lpszClassName = name ;
  wc.lpfnWndProc   = class_wndproc ;
  wc.hCursor       = NULL ;
  wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW ;
  wc.hInstance     = NULL ;
  wc.hIcon         = NULL;
  wc.hbrBackground = CreateSolidBrush(COLOR);
  wc.lpszMenuName  = NULL;

  return RegisterClassEx(&wc);
}

static void canvas_create(void)
{
  const int eflag = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT ;
  const int wflag = WS_VISIBLE | WS_POPUP ;

  const int xs = 1024 ;
  const int ys = 768 ;

  if(0 == class_register())
  {
  }

  if(NULL == (canvas = CreateWindowEx(eflag, name, name, wflag, CW_USEDEFAULT, CW_USEDEFAULT, xs, ys, NULL, NULL, NULL, NULL)))
  {
    assert(0);
  }

  if(0 == SetWindowPos(canvas, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW /* | SWP_NOACTIVATE */))
  {
    assert(0);
  }

  if(0 == SetLayeredWindowAttributes(canvas, COLOR, 128, /* LWA_COLORKEY | */ LWA_ALPHA))
  {
    assert(0);
  }

  printf("created SDK target window with handle %p\n", canvas);
}

static void canvas_pump(void)
{
  MSG msg ;
  int ret ;

  while(0 != (ret = GetMessage(&msg, canvas, 0, 0)))
  {
    if(-1 == ret)
    {
      break ;
    }

    if(WM_CLOSE == msg.message)
    {
      break ;
    }

    if(0 == TranslateMessage(&msg))
    {
    }

    if(0 == DispatchMessage(&msg))
    {
    }
  }
}

static void canvas_dispose(void)
{
  if(0 == DestroyWindow(canvas))
  {
  }
}

static void canvas_clear(void)
{
  if(0 == InvalidateRect(canvas, NULL, TRUE))
  {
  }
}

/**/

static void * user_create(int uid)
{
  user_t * this = malloc(sizeof * this) ;

  const int R = rand() % 255 ;
  const int G = rand() % 255 ;
  const int B = rand() % 255 ;

  this -> flag = FLAG_NONE ;

  this -> uid = uid;

  /* reverse RGB */

  this -> pen.rgb  = RGB(R, G, B);
  this -> pen.size = 10 ;

  this -> pos.x = 0 ;
  this -> pos.y = 0 ;

  return this ;
}

static void users_init(void)
{
  int i;

  for(i = 0; i < USER_MAX; i++)
  {
    user_t * this = user_create(i);

    this -> flag |= FLAG_START;

    users[i] = this ;
  }
}

static void users_deinit(void)
{
  int i;

  for(i = 0; i < USER_MAX; i++)
  {
    free(users[i]);
  }
}

static int dpi_aware_set(void)
{
  HMODULE dll = LoadLibrary("shcore.dll");

  if(NULL != dll)
  {
     /* win 8+ */

     HRESULT (WINAPI * proc)(int) = (HRESULT (WINAPI *)(int))GetProcAddress(dll, "SetProcessDpiAwareness");

     if(NULL != proc)
     {
       if(S_OK == proc(2 /* PROCESS_PER_MONITOR_DPI_AWARE */))
       {
         return 1;
       }
     }
  }

  if(NULL != (dll = LoadLibrary("user32.dll")))
  {
     /* win 7 */

     BOOL (WINAPI * proc)(void) = (BOOL (WINAPI *)(void))GetProcAddress(dll, "SetProcessDPIAware");

     if(NULL != proc)
     {
       if(0 != proc())
       {
         return 1;
       }
     }
  }

  return 0;
}

int main(void)
{
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  sdk_init();
  dpi_aware_set();
  users_init();

  canvas_create();
  canvas_pump();
  canvas_dispose();

  users_deinit();

  return 0;
}
