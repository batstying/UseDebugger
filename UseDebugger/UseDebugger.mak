# Microsoft Developer Studio Generated NMAKE File, Based on UseDebugger.dsp
!IF "$(CFG)" == ""
CFG=UseDebugger - Win32 Debug
!MESSAGE No configuration specified. Defaulting to UseDebugger - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "UseDebugger - Win32 Release" && "$(CFG)" != "UseDebugger - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UseDebugger.mak" CFG="UseDebugger - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UseDebugger - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "UseDebugger - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UseDebugger - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\UseDebugger.exe"


CLEAN :
	-@erase "$(INTDIR)\BaseEvent.obj"
	-@erase "$(INTDIR)\Common.obj"
	-@erase "$(INTDIR)\DllEvent.obj"
	-@erase "$(INTDIR)\ExceptEvent.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\ProcessEvent.obj"
	-@erase "$(INTDIR)\UI.OBJ"
	-@erase "$(INTDIR)\UseDebugger.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\UseDebugger.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\UseDebugger.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\UseDebugger.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\UseDebugger.pdb" /machine:I386 /out:"$(OUTDIR)\UseDebugger.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BaseEvent.obj" \
	"$(INTDIR)\Common.obj" \
	"$(INTDIR)\DllEvent.obj" \
	"$(INTDIR)\ExceptEvent.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ProcessEvent.obj" \
	"$(INTDIR)\UI.OBJ" \
	"$(INTDIR)\UseDebugger.obj" \
	".\Decode2Asm.obj" \
	".\Disasm.obj" \
	".\Dsasm_Functions.obj"

"$(OUTDIR)\UseDebugger.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\UseDebugger.exe" "$(OUTDIR)\UseDebugger.bsc"


CLEAN :
	-@erase "$(INTDIR)\BaseEvent.obj"
	-@erase "$(INTDIR)\BaseEvent.sbr"
	-@erase "$(INTDIR)\Common.obj"
	-@erase "$(INTDIR)\Common.sbr"
	-@erase "$(INTDIR)\DllEvent.obj"
	-@erase "$(INTDIR)\DllEvent.sbr"
	-@erase "$(INTDIR)\ExceptEvent.obj"
	-@erase "$(INTDIR)\ExceptEvent.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\ProcessEvent.obj"
	-@erase "$(INTDIR)\ProcessEvent.sbr"
	-@erase "$(INTDIR)\UI.OBJ"
	-@erase "$(INTDIR)\UI.SBR"
	-@erase "$(INTDIR)\UseDebugger.obj"
	-@erase "$(INTDIR)\UseDebugger.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\UseDebugger.bsc"
	-@erase "$(OUTDIR)\UseDebugger.exe"
	-@erase "$(OUTDIR)\UseDebugger.ilk"
	-@erase "$(OUTDIR)\UseDebugger.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\UseDebugger.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\UseDebugger.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BaseEvent.sbr" \
	"$(INTDIR)\Common.sbr" \
	"$(INTDIR)\DllEvent.sbr" \
	"$(INTDIR)\ExceptEvent.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\ProcessEvent.sbr" \
	"$(INTDIR)\UI.SBR" \
	"$(INTDIR)\UseDebugger.sbr"

"$(OUTDIR)\UseDebugger.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\UseDebugger.pdb" /debug /machine:I386 /out:"$(OUTDIR)\UseDebugger.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\BaseEvent.obj" \
	"$(INTDIR)\Common.obj" \
	"$(INTDIR)\DllEvent.obj" \
	"$(INTDIR)\ExceptEvent.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ProcessEvent.obj" \
	"$(INTDIR)\UI.OBJ" \
	"$(INTDIR)\UseDebugger.obj" \
	".\Decode2Asm.obj" \
	".\Disasm.obj" \
	".\Dsasm_Functions.obj"

"$(OUTDIR)\UseDebugger.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("UseDebugger.dep")
!INCLUDE "UseDebugger.dep"
!ELSE 
!MESSAGE Warning: cannot find "UseDebugger.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "UseDebugger - Win32 Release" || "$(CFG)" == "UseDebugger - Win32 Debug"
SOURCE=.\BaseEvent.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\BaseEvent.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\BaseEvent.obj"	"$(INTDIR)\BaseEvent.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Common.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\Common.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\Common.obj"	"$(INTDIR)\Common.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\DllEvent.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\DllEvent.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\DllEvent.obj"	"$(INTDIR)\DllEvent.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ExceptEvent.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\ExceptEvent.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\ExceptEvent.obj"	"$(INTDIR)\ExceptEvent.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\main.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ProcessEvent.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\ProcessEvent.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\ProcessEvent.obj"	"$(INTDIR)\ProcessEvent.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\UI.CPP

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\UI.OBJ" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\UI.OBJ"	"$(INTDIR)\UI.SBR" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\UseDebugger.cpp

!IF  "$(CFG)" == "UseDebugger - Win32 Release"


"$(INTDIR)\UseDebugger.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "UseDebugger - Win32 Debug"


"$(INTDIR)\UseDebugger.obj"	"$(INTDIR)\UseDebugger.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

