/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *                                                                           
 *  Version:    C, Version 2.1
 *                                                                           
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 ****************************************************************************
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1176ec
#define LOG_MODULE    AMOD_SYS

#if defined(__CC_ARM) /* Arm compiler 5 */

#elif defined(__ARMCC_VERSION)&&(__ARMCC_VERSION >= 6010050) /* Arm compiler6 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int"
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#pragma clang diagnostic ignored "-Wreturn-type"
#elif defined(__GNUC__) /* Normal GCC*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-int"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wreturn-type"
#else
 #error Unknown compiler
#endif

#define NUMBER_OF_RUNS  1000000

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <core_op.h>
#include "dhry.h"

/* Global Variables: */

Rec_Pointer     Ptr_Glob,
                Next_Ptr_Glob;
int             Int_Glob;
Boolean         Bool_Glob;
char            Ch_1_Glob,
                Ch_2_Glob;
int             Arr_1_Glob [50];
int             Arr_2_Glob [50] [50];

Enumeration     Func_1 ();

/* forward declaration necessary since Enumeration may not simply be int */
 
#ifndef REG
        Boolean Reg = false;
#define REG
        /* REG becomes defined as empty */
        /* i.e. no register variables   */
#else
        Boolean Reg = true;
#endif
u64 Begin_Time, End_Time, User_Time;

/* end of variables for time measurement */

void dhry_test(void)
/*****/

  /* main program, corresponds to procedures        */
  /* Main and Proc_0 in the Ada version             */
{
        One_Fifty       Int_1_Loc;
  REG   One_Fifty       Int_2_Loc;
        One_Fifty       Int_3_Loc;
  REG   char            Ch_Index;
        Enumeration     Enum_Loc;
        Str_30          Str_1_Loc;
        Str_30          Str_2_Loc;
  REG   int             Run_Index;
  REG   int             Number_Of_Runs;
  AIC_INT_SAVE_AREA
  /* Initializations */
  /*
  Next_Ptr_Glob = (Rec_Pointer) rt_malloc (sizeof (Rec_Type));
  Ptr_Glob = (Rec_Pointer) rt_malloc (sizeof (Rec_Type));
*/
  Next_Ptr_Glob = (Rec_Pointer) aos_malloc (sizeof(Rec_Type));
  Ptr_Glob = (Rec_Pointer) aos_malloc (sizeof(Rec_Type));

  Ptr_Glob->Ptr_Comp                    = Next_Ptr_Glob;
  Ptr_Glob->Discr                       = Ident_1;
  Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
  Ptr_Glob->variant.var_1.Int_Comp      = 40;
  strncpy (Ptr_Glob->variant.var_1.Str_Comp,
          "DHRYSTONE PROGRAM, SOME STRING", sizeof(Ptr_Glob->variant.var_1.Str_Comp));
  strncpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING", sizeof(Str_1_Loc));

  Arr_2_Glob [8][7] = 10;
        /* Was missing in published program. Without this statement,    */
        /* Arr_2_Glob [8][7] would have an undefined value.             */
        /* Warning: With 16-Bit processors and Number_Of_Runs > 32000,  */
        /* overflow may occur for this array element.                   */

  alog_info("***********************************************************\n");
  alog_info("\n");
  alog_info("Dhrystone Benchmark, Version 2.1 (Language: C)\n");
  alog_info("\n");
#if 0
  if (Reg)
  {
    aicx_info(0x121dc1, "", "Program compiled with 'register' attribute\n");
    aicx_info(0x1c0b11, "", "\n");
  }
  else
  {
    alog_info("Program compiled without 'register' attribute\n");
    alog_info("\n");
  }
  alog_info("Please give the number of runs through the benchmark: ");
#endif
  Number_Of_Runs = NUMBER_OF_RUNS;
#if 0
  alog_info("%d\n", Number_Of_Runs);

  alog_info("\n");

  alog_info("Execution starts, %d runs through Dhrystone\n", Number_Of_Runs);
#endif
  /***************/
  /* Start timer */
  /***************/
 
// Add your timer initializing code here
  Begin_Time = aos_get_tickcount(); /* get start tick */
  AIC_INT_DISABLE
  for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
  {

    Proc_5();
    Proc_4();
      /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
    Int_1_Loc = 2;
    Int_2_Loc = 3;
    strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
    Enum_Loc = Ident_2;
    Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
      /* Bool_Glob == 1 */
    while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
    {
      Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
        /* Int_3_Loc == 7 */
      Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
        /* Int_3_Loc == 7 */
      Int_1_Loc += 1;
    } /* while */
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
      /* Int_Glob == 5 */
    Proc_1 (Ptr_Glob);
    for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                             /* loop body executed twice */
    {
      if (Enum_Loc == Func_1 (Ch_Index, 'C'))
          /* then, not executed */
        {
        Proc_6 (Ident_1, &Enum_Loc);
        strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
        Int_2_Loc = Run_Index;
        Int_Glob = Run_Index;
        }
    }
      /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    Int_2_Loc = Int_2_Loc * Int_1_Loc;
    Int_1_Loc = Int_2_Loc / Int_3_Loc;
    Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
      /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
    Proc_2 (&Int_1_Loc);
      /* Int_1_Loc == 5 */

  } /* loop "for Run_Index" */
  AIC_INT_RESTORE

  /**************/
  /* Stop timer */
  /**************/
  End_Time = aos_get_tickcount(); // Get end tick

#if 0
  alog_info("Execution ends\n");
  alog_info("\n");
  alog_info("Final values of the variables used in the benchmark:\n");
  alog_info("\n");
  alog_info("Int_Glob:            %d\n", Int_Glob);
  alog_info("        should be:   %d\n", 5);
  alog_info("Bool_Glob:           %d\n", Bool_Glob);
  alog_info("        should be:   %d\n", 1);
  alog_info("Ch_1_Glob:           %c\n", Ch_1_Glob);
  alog_info("        should be:   %c\n", 'A');
  alog_info("Ch_2_Glob:           %c\n", Ch_2_Glob);
  alog_info("        should be:   %c\n", 'B');
  alog_info("Arr_1_Glob[8]:       %d\n", Arr_1_Glob[8]);
  alog_info("        should be:   %d\n", 7);
  alog_info("Arr_2_Glob[8][7]:    %d\n", Arr_2_Glob[8][7]);
  alog_info("        should be:   Number_Of_Runs + 10\n");
  alog_info("Ptr_Glob->\n");
  alog_info("  Ptr_Comp:          %d\n", (int) Ptr_Glob->Ptr_Comp);
  alog_info("        should be:   (implementation-dependent)\n");
  alog_info("  Discr:             %d\n", Ptr_Glob->Discr);
  alog_info("        should be:   %d\n", 0);
  alog_info("  Enum_Comp:         %d\n", Ptr_Glob->variant.var_1.Enum_Comp);
  alog_info("        should be:   %d\n", 2);
  alog_info("  Int_Comp:          %d\n", Ptr_Glob->variant.var_1.Int_Comp);
  alog_info("        should be:   %d\n", 17);
  alog_info("  Str_Comp:          %s\n", Ptr_Glob->variant.var_1.Str_Comp);
  alog_info("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
  alog_info("Next_Ptr_Glob->\n");
  alog_info("  Ptr_Comp:          %d\n", (int) Next_Ptr_Glob->Ptr_Comp);
  alog_info("        should be:   (implementation-dependent), same as above\n");
  alog_info("  Discr:             %d\n", Next_Ptr_Glob->Discr);
  alog_info("        should be:   %d\n", 0);
  alog_info("  Enum_Comp:         %d\n", Next_Ptr_Glob->variant.var_1.Enum_Comp);
  alog_info("        should be:   %d\n", 1);
  alog_info("  Int_Comp:          %d\n", Next_Ptr_Glob->variant.var_1.Int_Comp);
  alog_info("        should be:   %d\n", 18);
  alog_info("  Str_Comp:          %s\n",
                                Next_Ptr_Glob->variant.var_1.Str_Comp);
  alog_info("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
  alog_info("Int_1_Loc:           %d\n", Int_1_Loc);
  alog_info("        should be:   %d\n", 5);
  alog_info("Int_2_Loc:           %d\n", Int_2_Loc);
  alog_info("        should be:   %d\n", 13);
  alog_info("Int_3_Loc:           %d\n", Int_3_Loc);
  alog_info("        should be:   %d\n", 7);
  alog_info("Enum_Loc:            %d\n", Enum_Loc);
  alog_info("        should be:   %d\n", 1);
  alog_info("Str_1_Loc:           %s\n", Str_1_Loc);
  alog_info("        should be:   DHRYSTONE PROGRAM, 1'ST STRING\n");
  alog_info("Str_2_Loc:           %s\n", Str_2_Loc);
  alog_info("        should be:   DHRYSTONE PROGRAM, 2'ND STRING\n");
  alog_info("\n");
#endif
  User_Time = End_Time - Begin_Time;

  alog_info("mips = Dhrystones_Per_Second / 1757\n");
  alog_info("Number_of_Runs = %d, User_Time = %ldms\n",
            Number_Of_Runs, User_Time);
  alog_info("***********************************************************\n");
}

Proc_1 (Ptr_Val_Par)
/******************/

REG Rec_Pointer Ptr_Val_Par;
    /* executed once */
{
  REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp; 
                                        /* == Ptr_Glob_Next */
  /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
  /* corresponds to "rename" in Ada, "with" in Pascal           */
 
  structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
  Ptr_Val_Par->variant.var_1.Int_Comp = 5;
  Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
  Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
  Proc_3 (&Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
  if (Next_Record->Discr == Ident_1)
    /* then, executed */
  {
    Next_Record->variant.var_1.Int_Comp = 6;
    Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp,
           &Next_Record->variant.var_1.Enum_Comp);
    Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
    Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
           &Next_Record->variant.var_1.Int_Comp);
  }
  else /* not executed */
    structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */


Proc_2 (Int_Par_Ref)
/******************/
    /* executed once */
    /* *Int_Par_Ref == 1, becomes 4 */

One_Fifty   *Int_Par_Ref;
{
  One_Fifty  Int_Loc; 
  Enumeration   Enum_Loc;

  Int_Loc = *Int_Par_Ref + 10;
  do /* executed once */
    if (Ch_1_Glob == 'A')
      /* then, executed */
    {
      Int_Loc -= 1;
      *Int_Par_Ref = Int_Loc - Int_Glob;
      Enum_Loc = Ident_1;
    } /* if */
  while (Enum_Loc != Ident_1); /* true */
} /* Proc_2 */


Proc_3 (Ptr_Ref_Par)
/******************/
    /* executed once */
    /* Ptr_Ref_Par becomes Ptr_Glob */

Rec_Pointer *Ptr_Ref_Par;

{
  if (Ptr_Glob != Null)
    /* then, executed */
    *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */


Proc_4 () /* without parameters */
/*******/
    /* executed once */
{
  Boolean Bool_Loc;

  Bool_Loc = Ch_1_Glob == 'A';
  Bool_Glob = Bool_Loc | Bool_Glob;
  Ch_2_Glob = 'B';
} /* Proc_4 */


Proc_5 () /* without parameters */
/*******/
    /* executed once */
{
  Ch_1_Glob = 'A';
  Bool_Glob = false;
} /* Proc_5 */


        /* Procedure for the assignment of structures,          */
        /* if the C compiler doesn't support this feature       */
#ifdef  NOSTRUCTASSIGN
memcpy (d, s, l)
register char   *d;
register char   *s;
register int    l;
{
        while (l--) *d++ = *s++;
}
#endif

#if defined(__CC_ARM)
#elif defined(__ARMCC_VERSION)&&(__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#else
 #error Unknown compiler
#endif

