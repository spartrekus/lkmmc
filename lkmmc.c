/////////////////////////////////////////
//////////////////////////////////////////
// lkmmc (improved2, with colors)
//////////////////////////////////////////
//////////////////////////////////////////
#include <stdio.h>
#define PATH_MAX 2500
#if defined(__linux__) //linux
#define MYOS 1
#elif defined(_WIN32)
#define MYOS 2
#elif defined(_WIN64)
#define MYOS 3
#elif defined(__unix__) 
#define MYOS 4  // freebsd
#define PATH_MAX 2500
#else
#define MYOS 0
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>  




#define ESC "\033"
#define home() 			printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		printf(ESC "[2J") //clear the screen, move to (1,1)
#define gotoxy(x,y)		printf(ESC "[%d;%dH", y, x);
#define ansigotoyx(y,x)		printf(ESC "[%d;%dH", y, x);



int  rows, cols ; 
int  pansel = 1;
int  show_path_title = 1; 
char pathclipboard[3][PATH_MAX];
char clipboard_filter[PATH_MAX];
char file_filter[3][PATH_MAX];
int  scrollyclipboard[3] ;
int  selclipboard[3] ;


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"



char *fbasename(char *name)
{
  char *base = name;
  while (*name)
    {
      if (*name++ == '/')
	{
	  base = name;
	}
    }
  return (base);
}





///////////////////////////////////////////
void readfile( char *filesource )
{
   FILE *source; 
   int ch ; 
   source = fopen( filesource , "r");
   if ( source == NULL ) { printf( "File not found.\n" ); } else {
   while( ( ch = fgetc(source) ) != EOF )
   {
         printf( "%c", ch );
   }
   fclose(source);
   }
}
   









void nsystem( char *mycmd )
{
   printf( "<SYSTEM>\n" );
   printf( " >> CMD:%s\n", mycmd );
   system( mycmd );
   printf( "</SYSTEM>\n");
}





void nrunwith( char *cmdapp, char *filesource )
{
           char cmdi[PATH_MAX];
           strncpy( cmdi , "  " , PATH_MAX );
           strncat( cmdi , cmdapp , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , " " , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , " \"" , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi ,  filesource , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , "\" " , PATH_MAX - strlen( cmdi ) -1 );
           nsystem( cmdi ); 
}



void clear_screen()
{
    int fooi;
    struct winsize w; // need ioctl and unistd 
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
    clrscr();
    home();
}



int fexist(char *a_option)
{
  char dir1[PATH_MAX]; 
  char *dir2;
  DIR *dip;
  strncpy( dir1 , "",  PATH_MAX  );
  strncpy( dir1 , a_option,  PATH_MAX  );

  struct stat st_buf; 
  int status; 
  int fileordir = 0 ; 

  status = stat ( dir1 , &st_buf);
  if (status != 0) {
    fileordir = 0;
  }

  // this is compatible to check if a file exists
  FILE *fp2check = fopen( dir1  ,"r");
  if( fp2check ) {
  // exists
  fileordir = 1; 
  fclose(fp2check);
  } 

  if (S_ISDIR (st_buf.st_mode)) {
    fileordir = 2; 
  }
return fileordir;
/////////////////////////////
}







static struct termios oldt;

void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void enable_waiting_for_enter(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}


void nls()
{ 
   DIR *dirp;
   struct dirent *dp;
   dirp = opendir( "." );
   while  ((dp = readdir( dirp )) != NULL ) 
   {
         if (  strcmp( dp->d_name, "." ) != 0 )
         if (  strcmp( dp->d_name, ".." ) != 0 )
             printf( "%s\n", dp->d_name );
   }
   closedir( dirp );
}




/////////////////////////
/////////////////////////
int  nexp_user_sel[5] ; 
int  nexp_user_scrolly[5] ;
char nexp_user_fileselection[PATH_MAX]; 
int  tc_det_dir_type = 1;
/////////////////////////
void printdir( int pyy, int fopxx, char *mydir , int panviewpr )
{
   //int pxx = fopxx ;
   //if ( pxx == 0 ) pxx = 2;
   int pxx = fopxx;
   if ( pxx == 0 ) pxx = 2;
   //if ( panviewpr == 2 ) pxx = cols/2;
   DIR *dirp; int posy = 0;  int posx, chr ; 
   int fooselection = 0;

   posy = 1; posx = cols/2;

   char cwd[PATH_MAX];
   struct dirent *dp;
   dirp = opendir( mydir  );
   int entrycounter = 0;
   fooselection = 0;
   while  ((dp = readdir( dirp )) != NULL ) 
   if ( posy <= rows-3 )
   {
        entrycounter++;
        if ( entrycounter <= nexp_user_scrolly[panviewpr] )
              continue;

        if ( strcmp(  file_filter[panviewpr] , "" ) != 0 ) 
        {
           if ( strstr( dp->d_name, file_filter[panviewpr] ) == 0 ) 
              continue;
        }

        printf("%s", KNRM);


        if (  dp->d_name[0] !=  '.' ) 
        if (  strcmp( dp->d_name, "." ) != 0 )
        if (  strcmp( dp->d_name, ".." ) != 0 )
        {
            posy++;  fooselection++;
            if ( dp->d_type == DT_DIR ) 
            {
                 ansigotoyx( posy, pxx );
                 printf( "/" );
                 posx++;
                 printf("%s", KYEL);
            }
            else if ( dp->d_type == 0 )
            {
               if ( tc_det_dir_type == 1 )
               if ( fexist( dp->d_name ) == 2 )
               {
                 ansigotoyx( posy, pxx );
                 printf( "/" );
                 posx++;
                 printf("%s", KCYN);
               }
            }

            if ( nexp_user_sel[ panviewpr ] == fooselection ) 
            {
                  if ( panviewpr == pansel )
                  {
                    ansigotoyx( posy, pxx-1 );
                    //ansigotoyx( posy, fopxx );
                    strncpy( nexp_user_fileselection, dp->d_name , PATH_MAX );
                    printf( ">" );
                  }
            }
            else 
            {
                  ansigotoyx( posy, fopxx );
                  printf( " " );
            }


            ansigotoyx( posy, pxx );
            for ( chr = 0 ;  chr <= strlen(dp->d_name) ; chr++) 
            {
              if  ( dp->d_name[chr] == '\n' )
              {    //posx = cols/2;
              }
              else if  ( dp->d_name[chr] == '\0' )
              {    //posx = cols/2;
              }
              else
              {  
                 //mvaddch( posy, posx++ , dp->d_name[chr] );
                 printf( "%c", dp->d_name[chr] );
                 posx++;
              }
            }
        }
   }
   closedir( dirp );
   printf("%s", KNRM);
   // color_set( 0, NULL ); attroff( A_REVERSE );
   //mvprintw( rows-1, cols/2, "[FILE: %s]", nexp_user_fileselection );
}





char userstr[PATH_MAX];
/////////////////////////////////////
void strninput( char *mytitle, char *foostr )
{
      strncpy( userstr , "" , PATH_MAX );
      disable_waiting_for_enter();
      char strmsg[PATH_MAX];
      char charo[PATH_MAX];
      int foousergam = 0; int ch ;  int chr;

      strncpy( strmsg, ""  ,  PATH_MAX );
      strncpy( strmsg, foostr , PATH_MAX );

      int j; 
      char ptr[PATH_MAX];
      char str[PATH_MAX];

      while( foousergam == 0 ) 
      {

         ansigotoyx( rows, 0 );
         for ( chr = 0 ;  chr <= cols-1 ; chr++) printf( " ");
         ansigotoyx( rows, 0 );
         printf( ": %s", strmsg );

         ch = getchar();
         if ( ch == 10 )            foousergam = 1;

	 else if ( ch == 27 ) 
	      strncpy( strmsg, ""  ,  PATH_MAX );

	 else if ( ch == 2 ) 
	      strncpy( strmsg, ""  ,  PATH_MAX );

	 else if ( ch == 4 ) 
	 {      
            snprintf( charo, PATH_MAX , "%s%d",  strmsg, (int)time(NULL));
	    strncpy( strmsg,  charo ,  PATH_MAX );
         }

	 else if ( ( ch == 8 )  || ( ch == 127 ) )  
         {
            if ( strlen( strmsg ) >= 2 ) 
            {
              j = 0; strncpy(  ptr , "" ,  PATH_MAX );
              for ( chr = 0 ;  chr <= strlen( strmsg )-2 ; chr++) 
              {
                 ptr[j++] = strmsg[chr];
              }
	      strncpy( strmsg, ptr  ,  PATH_MAX );
            }
            else
	      strncpy( strmsg, ""  ,  PATH_MAX );
         }

	 else if (
			(( ch >= 'a' ) && ( ch <= 'z' ) ) 
		        || (( ch >= 'A' ) && ( ch <= 'Z' ) ) 
		        || (( ch >= '1' ) && ( ch <= '9' ) ) 
		        || (( ch == '0' ) ) 
		        || (( ch == '~' ) ) 
		        || (( ch == '!' ) ) 
		        || (( ch == '&' ) ) 
		        || (( ch == '=' ) ) 
		        || (( ch == ':' ) ) 
		        || (( ch == ';' ) ) 
		        || (( ch == '<' ) ) 
		        || (( ch == '>' ) ) 
		        || (( ch == ' ' ) ) 
		        || (( ch == '|' ) ) 
		        || (( ch == '#' ) ) 
		        || (( ch == '?' ) ) 
		        || (( ch == '+' ) ) 
		        || (( ch == '/' ) ) 
		        || (( ch == '\\' ) ) 
		        || (( ch == '.' ) ) 
		        || (( ch == '$' ) ) 
		        || (( ch == '%' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == ',' ) ) 
		        || (( ch == '{' ) ) 
		        || (( ch == '}' ) ) 
		        || (( ch == '(' ) ) 
		        || (( ch == ')' ) ) 
		        || (( ch == ']' ) ) 
		        || (( ch == '[' ) ) 
		        || (( ch == '*' ) ) 
		        || (( ch == '"' ) ) 
		        || (( ch == '@' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == '_' ) ) 
		        || (( ch == '^' ) ) 
		        || (( ch == '\'' ) ) 
	             ) 
		  {
                        snprintf( charo, PATH_MAX , "%s%c",  strmsg, ch );
		        strncpy( strmsg,  charo ,  PATH_MAX );
		  }
     }
     strncpy( userstr, strmsg , PATH_MAX );
}



///////////////////////////////////////////
void printhline( )
{
   int ch; 
   for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
}







///////////////////////////////////////////
void readfilesp( char *filesource, int linestart , int lineend )
{
  FILE *source; 
  int ch ;  int linecount = 1;
  if ( fexist( filesource ) == 1 )
  {
    source = fopen( filesource , "r");
    //if ( source == NULL ) { printf( "File not found.\n" ); } else  
    {
     clear_screen();
     for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
     printf( "FILE: %s\n", filesource );
     for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
     printf( "%d: ", linecount );
     while( ( ch = fgetc(source) ) != EOF )
     {
         if ( linecount <= lineend ) 
         {
           if ( ch == '\n' ) 
           {
              linecount++; 
              printf( "\n%d: ", linecount );
           }
           else
              printf( "%c", ch );
         }
     }
     fclose(source);
     }
  
     printf( "\33[2K" ); 
     printf( "\r" );
     for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
   }
}
   






void printat( int y1, int x1, char *mystring )
{
         ansigotoyx( y1 , x1 );  
         printf( "%s", mystring );
}
void mvcenter( int myposypass, char *mytext )
{
      printat( myposypass , cols/2 - strlen( mytext )/2  , mytext );
}
void gfxhline( int y1 , int x1 , int x2 , int mychar )
{
    int foo, fooy , foox ;
    foo = x1;
    ansigotoyx( y1 , x1 );  
    for( foox = x1 ; foox <= x2 ; foox++) 
         printf( "%c", mychar );
}

void gfxrect( int y1, int x1, int y2, int x2 )
{
    int foo, fooy , foox ;
    for( foox = x1 ; foox <= x2 ; foox++) 
    for( fooy = y1 ; fooy <= y2 ; fooy++) 
    {
        ansigotoyx( fooy , foox );  
        printf( " " );
    }
}





void gfxframe( int y1, int x1, int y2, int x2 )
{
    int foo, fooy , foox ;


    foox = x1;
    for( fooy = y1 ; fooy <= y2 ; fooy++) 
    {
        ansigotoyx( fooy , foox );  
        printf(  "|" );
    }
   
    for( fooy = y1 ; fooy <= y2 ; fooy++) 
    {
        ansigotoyx( fooy , x1 );  
        //printf(ESC "[%d;%dH", fooy, x1 );
        printf( "|" );
    }

    foo = x2;
    for( fooy = y1 ; fooy <= y2 ; fooy++) 
    {
         ansigotoyx( fooy , foo );  
         printf( "|" );
    }
    foo = y1;
    for( foox = x1 ; foox <= x2 ; foox++) 
    {
         ansigotoyx( foo , foox );  
         printf( "-" );
    }
    foo = y2;
    for( foox = x1 ; foox <= x2 ; foox++) 
    {
         ansigotoyx( foo , foox );  
         printf( "-" );
    }
}





///////////////// new
char *fextension(char *str)
{ 
    char ptr[strlen(str)+1];
    int i,j=0;
    //char ptrout[strlen(ptr)+1];  
    char ptrout[25];

    if ( strstr( str, "." ) != 0 )
    {
      for(i=strlen(str)-1 ; str[i] !='.' ; i--)
      {
        if ( str[i] != '.' ) 
            ptr[j++]=str[i];
      } 
      ptr[j]='\0';

      j = 0; 
      for( i=strlen(ptr)-1 ;  i >= 0 ; i--)
            ptrout[j++]=ptr[i];
      ptrout[j]='\0';
    }
    else
     ptrout[0]='\0';

    size_t siz = sizeof ptrout ; 
    char *r = malloc( sizeof ptrout );
    return r ? memcpy(r, ptrout, siz ) : NULL;
}









////////////////////////////////////////
int main( int argc, char *argv[])
{
    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "-yellow" ) ==  0 ) 
    {
       printf("%syellow\n", KYEL);
       return 0;
    }

    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "-green" ) ==  0 ) 
    {
       printf("%sgreen\n", KGRN);
       return 0;
    }



     ////////////////////////////////////////////////////////
     if ( argc == 2)
     if ( strcmp( argv[1] , "-t" ) ==  0 ) 
     {
       printf("%d\n", (int)time(NULL));
       return 0;
     }


    struct winsize w; // need ioctl and unistd 
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
    char string[PATH_MAX];
    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "-size" ) ==  0 ) 
    {
              printf("Screen\n" );
              printf("Env HOME:  %s\n", getenv( "HOME" ));
              printf("Env PATH:  %s\n", getcwd( string, PATH_MAX ) );
              printf("Env TERM ROW:  %d\n", w.ws_row );
              printf("Env TERM COL:  %d\n", w.ws_col );
              return 0;
    }    


    int viewpan[5];
    viewpan[ 1 ] = 1;
    viewpan[ 2 ] = 1;
    ////////////////////////////////////////////////////////
    char cwd[PATH_MAX];
    char pathpan[5][PATH_MAX];



     ////////////////////////////////////////////////////////
     if ( argc == 2)
     if ( strcmp( argv[1] , "-1" ) ==  0 ) 
     {
           viewpan[ 1 ] = 1;
           viewpan[ 2 ] = 0;
     }

     ////////////////////////////////////////////////////////
     if ( argc == 3)
     if ( strcmp( argv[1] , "-1" ) ==  0 ) 
     {
          viewpan[ 1 ] = 1;
          viewpan[ 2 ] = 0;
          chdir( argv[ 2 ] );
          strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
          strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
     }

     ////////////////////////////////////////////////////////
     // filed
     if ( argc == 3)
     if ( strcmp( argv[1] , "-f" ) ==  0 ) 
     {
       printf("%syellow\n", KYEL);
       readfile( argv[ 2 ] );
       return 0;
     }


    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "" ) !=  0 )
    if ( fexist( argv[1] ) ==  2 )
    {
          chdir( argv[ 1 ] );
          strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
          strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
    }




    ////////////////////////////////////////////////////////
    nexp_user_sel[1] = 1;
    nexp_user_sel[2] = 1;
    nexp_user_scrolly[1] = 0;
    nexp_user_scrolly[2] = 0;
    strncpy( pathpan[ 1 ] ,  getcwd( cwd, PATH_MAX ), PATH_MAX );
    strncpy( pathpan[ 2 ] ,  getcwd( cwd, PATH_MAX ), PATH_MAX );
    strncpy( pathclipboard[1] , getcwd( cwd, PATH_MAX ), PATH_MAX );
    strncpy( pathclipboard[2] , getcwd( cwd, PATH_MAX ), PATH_MAX );
    strncpy( file_filter[1] ,   "" , PATH_MAX );
    strncpy( file_filter[2] ,   "" , PATH_MAX );
    strncpy( clipboard_filter , "" , PATH_MAX );






    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "-s" ) ==  0 ) 
    {
       printf("Screen\n" );
       printf("Env HOME:  %s\n", getenv( "HOME" ));
       printf("Env PATH:  %s\n", getcwd( string, PATH_MAX ) );
       printf("Env TERM ROW:  %d\n", w.ws_row );
       printf("Env TERM COL:  %d\n", w.ws_col );
       return 0;
    }
    rows = w.ws_row ; 
    cols = w.ws_col ; 


    int ch ; 
    int gameover = 0;
    int foo;
    char userstrsel[PATH_MAX];
    char lkmmc_message[PATH_MAX];
    strncpy( lkmmc_message, "" , PATH_MAX );


    ////////////////////////////////////////////////////////
    if ( argc == 3)
    //if ( fexist( argv[1] ) == 2 ) 
    //if ( fexist( argv[2] ) == 2 ) 
    {
          viewpan[ 1 ] = 1;
          viewpan[ 2 ] = 1;
          chdir( argv[ 1 ] );
          strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
          chdir( argv[ 2 ] );
          strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
          chdir( argv[ 1 ] );
    }


    printf( "|LKMMC|\n" );
    printf( "|*1 |[%s]\n", pathpan[ 1 ] );
    printf( "| 2 |[%s]\n", pathpan[ 2 ] );
    printf( "Reading directories...\n" );

    while ( gameover == 0 ) 
    {
       strncpy( nexp_user_fileselection, "" , PATH_MAX );
       disable_waiting_for_enter();
       clear_screen();

       ansigotoyx( 0, 0 );
       if ( show_path_title == 1 ) 
       {
         ansigotoyx( 0, 0 );
         if ( pansel == 1 )
           printf( "|*1 |[%s]", pathpan[ 1 ] );
         else 
           printf( "| 1 |[%s]", pathpan[ 1 ] );
  
         ansigotoyx( 0, cols/2 );
         if ( pansel == 2 )
           printf( "|*2 |[%s]", pathpan[ 2 ] );
         else 
           printf( "| 2 |[%s]", pathpan[ 2 ] );
       }

       chdir( pathpan[ 1 ] );
       if ( viewpan[ 1 ] == 1 ) 
          printdir( 0, 0,       "." , 1 );

       chdir( pathpan[ 2 ] );
       if ( viewpan[ 2 ] == 1 ) 
          printdir( 0, cols/2,  "." , 2 );

       ansigotoyx( rows-1, 0 );
       printf( "|%s|F|%d|[%s]", "LKMM" , nexp_user_sel[pansel] ,  nexp_user_fileselection );

       ansigotoyx( rows, 0 );
       printf( "| j:Down | k:Up | h:CdParent | l:CdSel | n:ScrollDown | u:ScrollUp | r:Multi |" );

       ch = getchar();

       chdir( pathpan[ pansel ] );

       if      (ch ==  'Q')      gameover = 1;
       else if (ch ==  'q')      gameover = 1;

       else if ( ch == 'r' ) 
       {  enable_waiting_for_enter();  
          if (  fexist(  nexp_user_fileselection    ) == 1 ) 
          {
             if ( fexist( "/usr/local/bin/lfview" ) == 1 ) 
              nrunwith(  " lfview ",  nexp_user_fileselection    );   
             else
              nrunwith(  " less ",  nexp_user_fileselection    );   
          }
       }

       ////////////////////////////
       else if ( ch == 15 ) 
       {
           strninput( " Change Directory (chdir) ", "" );
           strncpy( string, userstr , PATH_MAX );
           printf("\n" );
           printf("\n" );
           printf("got: \"%s\"\n", string );
           if ( strcmp( string , "" ) != 0 )
           {
               chdir( pathpan[ pansel ] );
               chdir( string ) ; 
               nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
               strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
           }
       }



       //////////////////////
       else if ( ch == 'O' ) 
       {
           strninput( " Change Directory (chdir) from HOME.", "" );
           strncpy( string, userstr , PATH_MAX );
           printf("\n" );
           printf("got: \"%s\"\n", string );
           if ( strcmp( string , "" ) != 0 )
           {
               chdir( getenv( "HOME" ) );
               chdir( string ); 
               nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
               strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
           }
       }




     // run it
     else if (  ( ch == 18 )    || ( ch == 5 ))   //c-r 18
     {
         strncpy( userstrsel, nexp_user_fileselection , PATH_MAX );
         ansigotoyx( rows-1 , 0 );
         printhline( );
         ansigotoyx( rows , 0 );
         printf( "Open menu...\n" );
         printhline( );
          
           printf("%s", KCYN);
           gfxrect(   rows*10/100 ,         cols*10/100, rows*90/100, cols*90/100 );
           gfxframe(  rows*10/100 ,         cols*10/100, rows*90/100, cols*90/100 );
           mvcenter(  rows*10/100+1, "| MENU LKMMC |");
           foo = 1;
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " e: ed ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " m: xmplayer ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " p: xmupdf ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " t: tless ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " l: less ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " v: vim ");
           printf("%s", KWHT);
         ch = getchar();
         enable_waiting_for_enter();
         printf( "\n" );
         printf( "Term!\n" );
         printf( "\n" );
         printf( "Key %d\n", ch );
         if ( ch == 'm') { printf( "mplayer\n" );  nrunwith( " export DISPLAY=:0 ; mplayer ", userstrsel ); }
         else if ( ch == 'e' )  nrunwith( " ed  ", userstrsel );
         else if ( ch == 't' )  nrunwith( " tless  ", userstrsel );
         else if ( ch == 'p' ) { printf( "mupdf\n" );  nrunwith( " export DISPLAY=:0 ;   mupdf ", userstrsel ); }
         else if ( ch == 'f' ) { printf( "feh\n" );  nrunwith( " export DISPLAY=:0 ; feh     ", userstrsel ); }
         else if ( ch == 'l' )  nrunwith( " less  ", userstrsel );
         ch = 0;
       }

       else if ( ch == 15 )   // working ctrl + o
       {
           strninput( " Change Directory (chdir) ", "" );
           strncpy( string, userstr , PATH_MAX );
           printf("\n" );
           printf("\n" );
           printf("got: \"%s\"\n", string );
           if ( strcmp( string , "" ) != 0 )
           {
               chdir( pathpan[ pansel ] );
               chdir( string ) ; 
               nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
               strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
           }
       }




      else if ( ch == 'z' )   {  enable_waiting_for_enter();  nrunwith(  " tless   ",  nexp_user_fileselection    );   }



      else if ( ch == 27 )  
      {
         ch = getchar();
         if ( ch == '[' )  
         {
           ch = getchar();
           if ( ch ==      66 )       system( "  export DISPLAY=:0 ;  xdotool  key Down " );
           else if ( ch == 68 )       system( "  export DISPLAY=:0 ;  xdotool  key Left " );
           else if ( ch == 67 )       system( "  export DISPLAY=:0 ;  xdotool  key Right " );
           else if ( ch == 65 )       system( "  export DISPLAY=:0 ;  xdotool  key Up " );
         }
      }

       else if ( ch == 'w')      
       {
            chdir( pathpan[ pansel ] );
            chdir( getenv( "HOME" ) );
            chdir( "workspace" );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
       }
       else if ( ch == '~')      
       {
            chdir( pathpan[ pansel ] );
            chdir( getenv( "HOME" ) );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
       }

       else if ( ch == '~')      
       {
            chdir( pathpan[ pansel ] );
            chdir( getenv( "HOME" ) );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
       }

       else if  ( ch == 's') 
       {
            enable_waiting_for_enter();
            clear_screen();
            printf( "========= \n" );
            printf( "= LKMMC = \n" );
            printf( "========= \n" );
            printf( " \n" );
            printf( " \n" );

            printf("PATH 1: %s \n", pathpan[ 1 ] );
            printf( " \n" );
            printf("PATH 2: %s \n", pathpan[ 2 ] );
            printf( " \n" );
            printf("File [%d]: %s \n", fexist(  nexp_user_fileselection    ) ,  nexp_user_fileselection    );
            printf( " \n" );

            //////////////////
            ansigotoyx( rows-1, 0 );
            for( foo = 0 ;  foo <= cols-1 ; foo++) printf( " " );
            ansigotoyx( rows-1, 0 ); printf( "<Press Key>" );
            disable_waiting_for_enter();
            getchar();
            //////////////////
        }


       else if ( ch == 'h')      
       {
            chdir( pathpan[ pansel ] );
            chdir( ".." );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
            strncpy( file_filter[pansel]  , "" , PATH_MAX );
       }
       else if ( ch == 'l')      
       {
            // save 
            chdir( pathpan[ pansel ] );
            strncpy( pathclipboard[pansel] , getcwd( string, PATH_MAX ), PATH_MAX );
            selclipboard[pansel] =      nexp_user_sel[pansel];
            scrollyclipboard[pansel] =  nexp_user_scrolly[pansel];
            strncpy( clipboard_filter , file_filter[pansel] , PATH_MAX );

            // go 
            chdir( pathpan[ pansel ] );
            chdir( nexp_user_fileselection );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
            strncpy( file_filter[pansel]  , "" , PATH_MAX );
       }


       /// FILE OPERATIONS BEGIN
       else if ( ch == '4') 
       {
                   chdir( pathpan[ pansel ] );
                   strncpy( string, " cp -a " , PATH_MAX );
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string ,   nexp_user_fileselection  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   strncat( string , "  " , PATH_MAX - strlen(string) - 1);
                   if ( pansel == 2 ) foo = 1; else if ( pansel == 1 ) foo = 2; 
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string , pathpan[ foo ] ,  PATH_MAX - strlen(string) - 1);
                   strncat( string , "/" ,  PATH_MAX - strlen(string) - 1);
                   strncat( string , fbasename(  nexp_user_fileselection ) , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   ansigotoyx( rows, 0 );
                   gfxhline( rows , 0 , cols-1, ' '); 
                   ansigotoyx( rows-1, 0 );
                   gfxhline( rows-1 , 0 , cols-1 , ' ' ); 
                   ansigotoyx( rows, 0 );
                   gfxhline(  rows-1 , 0 , cols-1 , '=' ); 
                   printf( "CMD: %s [y/n]?\n" ,  string );
                   printf( "Answer: Yes or No [y/n]?\n" );
                   printf( "=========================\n" );
                   foo = getchar();
                   if ( ( foo == '1' ) || ( foo == 'y' ) )
                   {   
                      printf( "=lkmm=================\n" );
                      printf( "%d\n", (int)time(NULL));
                      printf( "=====================\n" );
                      nsystem( string );
                      printf( "=====================\n" );
                      printf( "%d\n", (int)time(NULL));
                      printf( "=lkmm=================\n" );
                      printf( "Process Completed (%s).\n", string );
                      printf( "<Press Key>\n" );
                      printf( "=====================\n" );
                      getchar();
                   }
        }




       else if ( ch == '5') 
       {
                   chdir( pathpan[ pansel ] );
                   strncpy( string, " cp -r " , PATH_MAX );
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string ,   nexp_user_fileselection  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   strncat( string , "  " , PATH_MAX - strlen(string) - 1);
                   if ( pansel == 2 ) foo = 1; else if ( pansel == 1 ) foo = 2; 
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string , pathpan[ foo ] ,  PATH_MAX - strlen(string) - 1);
                   strncat( string , "/" ,  PATH_MAX - strlen(string) - 1);
                   strncat( string , fbasename(  nexp_user_fileselection ) , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   ansigotoyx( rows, 0 );
                   gfxhline( rows , 0 , cols-1, ' '); 
                   ansigotoyx( rows-1, 0 );
                   gfxhline( rows-1 , 0 , cols-1 , ' ' ); 
                   ansigotoyx( rows, 0 );
                   gfxhline(  rows-1 , 0 , cols-1 , '=' ); 
                   printf( "CMD: %s [y/n]?\n" ,  string );
                   printf( "Answer: Yes or No [y/n]?\n" );
                   printf( "=========================\n" );
                   foo = getchar();
                   if ( ( foo == '1' ) || ( foo == 'y' ) )
                      nsystem( string );
        }



        else if ( ch == '6') 
        {
                   chdir( pathpan[ pansel ] );
                   strncpy( string, " mv   " , PATH_MAX );
                   strncat( string , "   \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string ,   nexp_user_fileselection  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   strncat( string , "  " , PATH_MAX - strlen(string) - 1);
                   if ( pansel == 2 ) foo = 1; else if ( pansel == 1 ) foo = 2; 
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string , pathpan[ foo ] ,  PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   ansigotoyx( rows, 0 );
                   gfxhline( rows , 0 , cols-1, ' '); 
                   ansigotoyx( rows-1, 0 );
                   gfxhline( rows-1 , 0 , cols-1 , ' ' ); 
                   ansigotoyx( rows, 0 );
                   gfxhline(  rows-1 , 0 , cols-1 , '=' ); 
                   printf( "CMD: %s [y/n]?\n" ,  string );
                   printf( "Answer: Yes or No [y/n]?\n" );
                   printf( "=========================\n" );
                   foo = getchar();
                   if ( ( foo == '1' ) || ( foo == 'y' ) )
                      nsystem( string );
        }


        else if ( ch == '8') 
        {
                   ansigotoyx( rows-1 , 0 );
                   printhline( );
                   ansigotoyx( rows , 0 );
                   printf("%d\n", (int)time(NULL));
                   snprintf( string , PATH_MAX , "%d-doc.txt", (int)time(NULL));
                   strninput( "", string );
                   strncpy( string, userstr , PATH_MAX );
                   printf("got: \"%s\"\n", string );
                   chdir( pathpan[ pansel ] );
                   strncpy( string, " touch   " , PATH_MAX );
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string , userstr  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   strncat( string , "  " , PATH_MAX - strlen(string) - 1);

                   ansigotoyx( rows, 0 );
                   gfxhline( rows , 0 , cols-1, ' '); 
                   ansigotoyx( rows-1, 0 );
                   gfxhline( rows-1 , 0 , cols-1 , ' ' ); 
                   ansigotoyx( rows, 0 );
                   gfxhline(  rows-1 , 0 , cols-1 , '=' ); 

                   printf( "CMD: %s [y/n]?\n" ,  string );
                   printf( "Answer: Yes or No [y/n]?\n" );
                   printf( "=========================\n" );
                   foo = getchar();
                   if ( ( foo == '1' ) || ( foo == 'y' ) )
                      nsystem( string );
        }


        else if ( ( ch == '2') || ( ch == '3') )
        {
                   ansigotoyx( rows-1 , 0 );
                   printhline( );
                   ansigotoyx( rows , 0 );
                   strninput( "",    nexp_user_fileselection    );
                   strncpy( string, userstr , PATH_MAX );
                   printf("got: \"%s\"\n", string );

                   chdir( pathpan[ pansel ] );
                   strncpy( string, " mv  " , PATH_MAX );

                   strncat( string , "   \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string ,   nexp_user_fileselection  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);

                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string , userstr  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   strncat( string , "  " , PATH_MAX - strlen(string) - 1);

                   ansigotoyx( rows, 0 );
                   gfxhline( rows , 0 , cols-1, ' '); 
                   ansigotoyx( rows-1, 0 );
                   gfxhline( rows-1 , 0 , cols-1 , ' ' ); 
                   ansigotoyx( rows, 0 );
                   gfxhline(  rows-1 , 0 , cols-1 , '=' ); 

                   printf( "CMD: %s [y/n]?\n" ,  string );
                   printf( "Answer: Yes or No [y/n]?\n" );
                   printf( "=========================\n" );
                   foo = getchar();
                   if ( ( foo == '1' ) || ( foo == 'y' ) )
                      nsystem( string );
        }


        else if ( ch == '7') 
        {
                   ansigotoyx( rows-1 , 0 );
                   printhline( );
                   ansigotoyx( rows , 0 );
                   strninput( "", "" );
                   strncpy( string, userstr , PATH_MAX );
                   printf("got: \"%s\"\n", string );

                   chdir( pathpan[ pansel ] );
                   strncpy( string, " mkdir   " , PATH_MAX );
                   strncat( string , " \"" , PATH_MAX - strlen(string) - 1);
                   strncat( string , userstr  , PATH_MAX - strlen(string) - 1);
                   strncat( string , "\" " , PATH_MAX - strlen(string) - 1);
                   strncat( string , "  " , PATH_MAX - strlen(string) - 1);

                   ansigotoyx( rows, 0 );
                   gfxhline( rows , 0 , cols-1, ' '); 
                   ansigotoyx( rows-1, 0 );
                   gfxhline( rows-1 , 0 , cols-1 , ' ' ); 
                   ansigotoyx( rows, 0 );
                   gfxhline(  rows-1 , 0 , cols-1 , '=' ); 

                   printf( "CMD: %s [y/n]?\n" ,  string );
                   printf( "Answer: Yes or No [y/n]?\n" );
                   printf( "=========================\n" );
                   foo = getchar();
                   if ( ( foo == '1' ) || ( foo == 'y' ) )
                      nsystem( string );
        }
       /// FILE OPERATIONS END



        else if ( ch == 'm')   
        {
            chdir( pathpan[ pansel ] );
            strncpy( pathclipboard[pansel] , getcwd( string, PATH_MAX ), PATH_MAX );
            selclipboard[pansel] = nexp_user_sel[pansel];
            scrollyclipboard[pansel] = nexp_user_scrolly[pansel];
        }

        else if (  ch == 'b' ) 
        {
            home(); printf( "|BACK|");
            chdir( pathpan[ pansel ] );
            chdir( pathclipboard[pansel] );
            strncpy( pathpan[ pansel ] , getcwd( string, PATH_MAX ), PATH_MAX );
            strncpy( file_filter[pansel], clipboard_filter , PATH_MAX );
            nexp_user_sel[pansel] = 1;   nexp_user_scrolly[pansel] = 0; 
            nexp_user_sel[pansel] =      selclipboard[pansel] ;
            nexp_user_scrolly[pansel] =  scrollyclipboard[pansel] ;
        }





       else if ( ch == 10 ) 
       {
             if      ( strcmp( fextension( nexp_user_fileselection ) , "mp4" ) == 0 )
               nrunwith( " mplayer " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "png" ) == 0 )
               nrunwith( " feh  " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "pdf" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; screen -d -m  mupdf " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "wad" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; prboom-plus -iwad " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "PDF" ) == 0 )
               nrunwith( " mupdf " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "avi" ) == 0 )
               nrunwith( " mplayer " , nexp_user_fileselection );
             else if ( strcmp( fextension( nexp_user_fileselection ) , "MP4" ) == 0 )
               nrunwith( " mplayer " , nexp_user_fileselection );
             else if ( strcmp( fextension( nexp_user_fileselection ) , "mp3" ) == 0 )
               nrunwith( " mplayer " , nexp_user_fileselection );
             else if ( strcmp( fextension( nexp_user_fileselection ) , "wav" ) == 0 )
               nrunwith( " mplayer " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "jpg" ) == 0 )
               nrunwith( " feh  " , nexp_user_fileselection );
             else if ( strcmp( fextension( nexp_user_fileselection ) , "gif" ) == 0 )
               nrunwith( " feh  " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "doc" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; screen -d -m libreoffice " , nexp_user_fileselection );
             else if ( strcmp( fextension( nexp_user_fileselection ) , "xls" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; screen -d -m libreoffice " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "fig" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; screen -d -m  xfig " , nexp_user_fileselection );


             //else if ( strcmp( fextension( nexp_user_fileselection ) , "mrk" ) == 0 )
             //  nrunwith( " vim " , nexp_user_fileselection );
             //else if ( strcmp( fextension( nexp_user_fileselection ) , "bmr" ) == 0 )
             //  nrunwith( " vim " , nexp_user_fileselection );
             //else if ( strcmp( fextension( nexp_user_fileselection ) , "txt" ) == 0 )
             //  nrunwith( " vim " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "ws1" ) == 0 )
               nrunwith( " freelotus123 " , nexp_user_fileselection );

             else if ( strcmp( fextension( nexp_user_fileselection ) , "csv" ) == 0 )
               nrunwith( " plotcsv " , nexp_user_fileselection );

             else 
               nrunwith( " rox " , nexp_user_fileselection );
       }


       else if ( ch == 'p') 
       {
          //if ( fexist(   nexp_user_fileselection ) == 1 )
          {
            readfilesp( nexp_user_fileselection , 0 , rows-4 );
            getchar();
          }
       }



       else if ( ( ch == 'o') && ( pansel == 1 )   )
       {
            chdir( pathpan[ 1 ] );
            chdir( nexp_user_fileselection );
            nexp_user_sel[ 2 ] = 1; 
            nexp_user_scrolly[ 2 ] = 0; 
            strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
       }

       else if ( ( ch == 'o') && ( pansel == 2 )   )
       {
            chdir( pathpan[ 2 ] );
            chdir( nexp_user_fileselection );
            nexp_user_sel[ 1 ] = 1; 
            nexp_user_scrolly[ 1 ] = 0; 
            strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
       }



       else if ( ch == 'k')      nexp_user_sel[pansel]--;
       else if ( ch == 'j')      nexp_user_sel[pansel]++;
       else if ( ch == 'g')      
       {
           ch = getchar();
           if ( ch == 'g' )
           { nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; }

           else if ( ch == '1' )
           {  if ( viewpan[ 1 ] == 1 )   viewpan[ 1 ] = 0; else viewpan[ 1 ] = 1; }
           else if ( ch == '2' )
           { if ( viewpan[ 2 ] == 1 )    viewpan[ 2 ] = 0; else viewpan[ 2 ] = 1; }

            else if ( ch == 't' ) 
            {
                if (   show_path_title == 0 ) 
                   show_path_title = 1;
                else
                   show_path_title = 0;
            }
            ch = 0;
       }
       else if ( ch == 'G')      { nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; }
       else if ( ch == 'u')      nexp_user_scrolly[pansel]-=4;
       else if ( ch == 'd')      nexp_user_scrolly[pansel]+=4;
       else if ( ch == 'n')      nexp_user_scrolly[pansel]+=4;

       else if ( ch == 'c' )     printf("%syellow\n", KYEL);
       else if ( ch == 'C' )     printf("%sgreen\n",  KGRN);

       else if ( ch == 't' ) 
       {
           clear_screen();
           strncpy( string , "  " , PATH_MAX );
           strncat( string , " lkview " , PATH_MAX - strlen( string ) -1 );
           strncat( string , " " , PATH_MAX - strlen( string ) -1 );
           strncat( string , " \"" , PATH_MAX - strlen( string ) -1 );
           strncat( string ,  nexp_user_fileselection , PATH_MAX - strlen( string ) -1 );
           strncat( string , "\" " , PATH_MAX - strlen( string ) -1 );
           nsystem( string );  
       }

       else if ( ch == 'T' ) 
       {
           if (   tc_det_dir_type == 0 ) 
              tc_det_dir_type = 1;
           else
              tc_det_dir_type = 0;
       }



       else if ( ch == 'v' )
       {  enable_waiting_for_enter();  nrunwith(  " vim  ",  nexp_user_fileselection    );   }


        else if ( ch == '!') 
        {
            strninput( " Run Cmd on File (!) ", "" );
            strncpy( string, userstr , PATH_MAX );
            printf("\n" );
            printf("\n" );
            printf("got: \"%s\"\n", string );
            if ( strcmp( string , "" ) != 0 ) 
            {
               nrunwith( string , nexp_user_fileselection  ) ; 
            }
        }


        else if ( ch == '$' )   // S like silent
        {
            strninput( " Run SH Command ", "" );
            strncpy( string, userstr , PATH_MAX );
            printf("\n" );
            printf("\n" );
            printf("got: \"%s\"\n", string );
            if ( strcmp( string , "" ) != 0 ) 
            {
               printf("run: \"%s\"\n", string );
               enable_waiting_for_enter();
               nsystem( string );
               disable_waiting_for_enter();
               //getchar();
            }
            ch = 0; 
        }

        else if ( ch == '&' ) 
        {
            strninput( " Run SH Command ", "" );
            strncpy( string, userstr , PATH_MAX );
            printf("\n" );
            printf("\n" );
            printf("got: \"%s\"\n", string );
            if ( strcmp( string , "" ) != 0 ) 
            {
               printf("run: \"%s\"\n", string );
               enable_waiting_for_enter();
               nsystem( string );
               disable_waiting_for_enter();
               printf("<Key Press>\n" );
               getchar();
            }
            ch = 0; 
        }


       else if ( ch == ';' ) 
       {
            strninput( " Run CHDIR Command ", "" );
            strncpy( string, userstr , PATH_MAX );
            printf("\n" );
            printf("\n" );
            printf("got: \"%s\"\n", string );
            chdir( pathpan[ pansel ] );
            chdir( string );
            strncpy( pathpan[ pansel ] , getcwd( string, PATH_MAX ), PATH_MAX );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
       }


       else if ( ch == 27 ) 
       {
          ch = getchar();
          if ( ch == 91 )  
          {
            ch = getchar();
            if ( ch == 49 )  
            {
              ch = getchar();
              if ( ch == 57 )  
              {
                 ch = getchar();
                 if ( ch == 126 )  
                 {
                     printf( "\n" );
                     printf( "F8\n" );
                     printf( "\n" );
                     //getchar();
                 }
              }
            }
          }
       }

       else if ( ch == 'M' ) 
       {
            clear_screen( );
            ansigotoyx( rows/2-1 , 0 );
            printhline( );
            ansigotoyx( rows/2 , 0 );
            printf( "Interpreter LKMMC Message \n" );
            ansigotoyx( rows/2+1 , 0 );
            printf("\n" );
            strninput( "", "" );
            strncpy( string, userstr , PATH_MAX );
            printf("got: \"%s\"\n", string );
            strncpy( lkmmc_message, string, PATH_MAX );
       }


       else if ( ch == ':' ) 
       {
            clear_screen( );
            ansigotoyx( rows/2-1 , 0 );
            printhline( );
            ansigotoyx( rows/2 , 0 );
            printf( "Interpreter and commands\n" );
            ansigotoyx( rows/2+1 , 0 );
            printf("\n" );
            strninput( "", "" );
            strncpy( string, userstr , PATH_MAX );
            printf("got: \"%s\"\n", string );


            printf("User cmd: \"%s\"\n", string );
            enable_waiting_for_enter();

            if ( strcmp( string, "size" ) == 0 )  
            {
              printf("Screen\n" );
              printf("Env HOME:  %s\n", getenv( "HOME" ));
              printf("Env PATH:  %s\n", getcwd( string, PATH_MAX ) );
              printf("Env TERM ROW:  %d\n", w.ws_row );
              printf("Env TERM COL:  %d\n", w.ws_col );
              getchar();
            }    

            else if ( strcmp( string, "mpall" ) == 0 )  
                nsystem(  "  export DISPLAY=:0 ; mplayer * " );

            else if ( strcmp( string, "xrox" ) == 0 )  
                nsystem(  "  export DISPLAY=:0 ; rox " );
            else if ( strcmp( string, "xfeh" ) == 0 )  
                nrunwith(  "  export DISPLAY=:0 ; feh -FZ  ",  nexp_user_fileselection    );
            else if ( strcmp( string, "xmupdf" ) == 0 )  
                nrunwith(  "  export DISPLAY=:0 ; mupdf ",  nexp_user_fileselection    );
            else if ( strcmp( string, "xmplayer" ) == 0 )  
                nrunwith(  "  export DISPLAY=:0 ; mplayer ",  nexp_user_fileselection    );

            else if ( strcmp( string, "cal" ) == 0 )  
            {
              nsystem( " cal "   );
              printf( "<Press Key>\n" );
              disable_waiting_for_enter(); 
              getchar(); 
            }    

            else if ( strcmp( string, "box" ) == 0 )  
            {
              printf("Box\n" );
              printf("Env HOME:  %s\n", getenv( "HOME" ));
              printf("Env PATH:  %s\n", getcwd( string, PATH_MAX ) );
              printf("Env TERM ROW:  %d\n", w.ws_row );
              printf("Env TERM COL:  %d\n", w.ws_col );
              gfxrect(   (int)  rows*30/100 ,  (int) cols*30/100, rows*70/100, cols*70/100 );
              gfxframe(  (int)  rows*30/100 ,  (int)  cols*30/100, rows*70/100, cols*70/100 );
              mvcenter(  (int)  rows*30/100,   "| MENU |");
              getchar();
            }    

            else if ( strcmp( string, "reboot" ) == 0 )  
                nsystem( " reboot " ); 
            else if ( strcmp( string, "REBOOT" ) == 0 )  
                nsystem( " reboot " ); 
            else if ( strcmp( string, "make" ) == 0 )  
                nsystem( " make " ); 

            else if ( strcmp( string, "rc" ) == 0 )  
            {
               enable_waiting_for_enter();
               if ( MYOS == 1 ) 
                  readfilesp( "/etc/hostname" , 0 , rows-4 );
               else 
                  readfilesp( "/etc/rc.conf" , 0 , rows-4 );
               disable_waiting_for_enter();
               getchar();
            }    

            else if ( strcmp( string, "key" ) == 0 )  
            {
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
                 ch = getchar(); printf( "\nKEY %d %c\n", ch , ch );
            }
       }



       /// tab
       else if ( ch == 9 )
       {  if ( pansel == 1 )   pansel = 2 ; else pansel = 1; }


       else if ( ch == 'x' ) 
       {
           if ( strcmp(  file_filter[pansel] , "" ) != 0 ) 
           {
              strncpy( file_filter[pansel]  , "" , PATH_MAX );
              nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
           }
       }
       else if ( ch == 'f' ) 
       {
            ansigotoyx( rows-1 , 0 );
            printhline( );
            ansigotoyx( rows , 0 );
            strninput( "", "" );
            strncpy( string, userstr , PATH_MAX );
            strncpy( file_filter[pansel]  , userstr , PATH_MAX );
            printf("got: \"%s\"\n", string );
            { nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; }
       }

       /// and key 'i'
       else if ( ch == 'i' )
       {  if ( pansel == 1 )   pansel = 2 ; else pansel = 1; }



       else if ( ch == '?' )
       {
           home(); 
           ansigotoyx( 1 , 0 );
           printhline( );

           home(); 
           ansigotoyx( 1 , 0 );
           mvcenter( 1 , lkmmc_message );

           printf("%s", KCYN);
           gfxrect(   rows*10/100 ,         cols*10/100, rows*90/100, cols*90/100 );
           gfxframe(  rows*10/100 ,         cols*10/100, rows*90/100, cols*90/100 );
           mvcenter(  rows*10/100+1, "| HELP LKMMC |");
           foo = 1;
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , "  v.0.14.1 ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , " ");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , "  q: Quit");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , "  r,t,v: view (less), lkview, vim file");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , "  !: run with (system command)");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , "  $: system command");
           printat(   rows*10/100+1+foo++ , cols*10/100+1 , "  :: internal command");
           printf("%s", KWHT);

            //////////////////
            ansigotoyx( rows-1, 0 );
            for( foo = 0 ;  foo <= cols-1 ; foo++) printf( " " );
            ansigotoyx( rows-1, 0 ); printf( "<Press Key>" );
            disable_waiting_for_enter();
            getchar();
            ch = 0;
            //////////////////
       }


       else if  ( ( ch == 'A') || ( ch == 'a') || ( ch == 'q' ) )
       {
            // small size menu
            gfxrect(      rows*30/100 , cols*30/100, rows*70/100, cols*70/100 );
            gfxframe(     rows*30/100 , cols*30/100, rows*70/100, cols*70/100 );
            mvcenter(     rows*30/100, "| MENU |");
            foo = 1;
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "x: xterm");
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "u: unidoc");
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "s: sunidoc");
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "t: make mrk/bmr");
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "m: make with texmaker");
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "c: nclock");
            printat(   rows*30/100 +foo++ , cols*30/100+1 , "Q: Quit LKMMC");
            ansigotoyx(  rows*70/100 , cols*70/100 );
            ch = getchar();
            ansigotoyx( rows-1, 0 );
            if           ( ch == 'Q' ) gameover = 1;
            else if      ( ch == 'q' ) gameover = 1;
            else if      ( ch == 's' ) nrunwith( "  sunidoc " , nexp_user_fileselection );
            else if      ( ch == 'u' ) nrunwith( "  unidoc " , nexp_user_fileselection );

            else if      ( ch == 't' ) nrunwith( "  makebmr  " , nexp_user_fileselection );
            else if      ( ch == 'm' ) nrunwith( "  texmaker " , nexp_user_fileselection );

            else if      ( ch == 'x' ) {  nsystem( " xterm " );  }
            else if      ( ch == 'c' ) {  nsystem( " nclock " );  }
            else if      ( ch == 'U' ) 
              nrunwith( "  export DISPLAY=:0 ; xunidoc " , nexp_user_fileselection );
            else if ( ch == '1' )
            {  if ( viewpan[ 1 ] == 1 )   viewpan[ 1 ] = 0; else viewpan[ 1 ] = 1; }
            else if ( ch == '2' )
            { if ( viewpan[ 2 ] == 1 )    viewpan[ 2 ] = 0; else viewpan[ 2 ] = 1; }
            ch = 0;
        }



    }

    enable_waiting_for_enter();
    printf( "\n" );
    printf( "Bye!\n" );
    return 0;
}


