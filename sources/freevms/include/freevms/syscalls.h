/*
================================================================================
  FreeVMS (R)
  Copyright (C) 2010 Dr. BERTRAND Joël and all.

  This file is part of FreeVMS

  FreeVMS is free software; you can redistribute it and/or modify it
  under the terms of the CeCILL V2 License as published by the french
  CEA, CNRS and INRIA.
 
  FreeVMS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the CeCILL V2 License
  for more details.
 
  You should have received a copy of the CeCILL License
  along with FreeVMS. If not, write to info@cecill.info.
================================================================================
*/

#define SYS$CALL(n)                 (n)

#define SYSCALL$NAME_SERVER         SYS$CALL(0b1)

#define SYSCALL$NEW_PROCESS         SYS$CALL(0b010)
#define SYSCALL$NEW_THREAD          SYS$CALL(0b011)
#define SYSCALL$KILL_PROCESS        SYS$CALL(0b100)
#define SYSCALL$KILL_THREAD         SYS$CALL(0b101)     // Untyped word (tid)

#define SYSCALL$PRINT               SYS$CALL(0b1000)    // StringItem

#define SYSCALL$EXIT_VALUE          SYS$CALL(0b10000)   // Untyped word (value)
