/*****
 ** ** Module Header ******************************************************* **
 ** 									     **
 **   Modules Revision 3.0						     **
 **   Providing a flexible user environment				     **
 ** 									     **
 **   File:		utility.c					     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Authors:	John Furlan, jlf@behere.com				     **
 **		Jens Hamisch, jens@Strawberry.COM			     **
 ** 									     **
 **   Description:	General routines that are called throughout Modules  **
 **			which are not necessarily specific to any single     **
 **			block of functionality.				     **
 ** 									     **
 **   Exports:		store_hash_value				     **
 **			clear_hash_value				     **
 **			Delete_Global_Hash_Tables			     **
 **			Delete_Hash_Tables				     **
 **			Copy_Hash_Tables				     **
 **			Unwind_Modulefile_Changes			     **
 **			Output_Modulefile_Changes			     **
 **			set_derelict					     **
 **			IsLoaded_ExactMatch				     **
 **			IsLoaded					     **
 **			chk_marked_entry				     **
 **			set_marked_entry				     **
 **			Update_LoadedList				     **
 **			ForceBasePath					     **
 **			ForceSacredPath					     **
 **			check_magic					     **
 **			cleanse_path					     **
 **			chk4spch					     **
 **			xdup						     **
 **			xgetenv						     **
 **									     **
 **			strdup		if not defined by the system libs.   **
 **			strtok		if not defined by the system libs.   **
 ** 									     **
 **   Notes:								     **
 ** 									     **
 ** ************************************************************************ **
 ****/

/** ** Copyright *********************************************************** **
 ** 									     **
 ** Copyright 1991-1994 by John L. Furlan.                      	     **
 ** see LICENSE.GPL, which must be provided, for details		     **
 ** 									     ** 
 ** ************************************************************************ **/

static char Id[] = "@(#)$Id: utility.c,v 1.1 2000/06/28 00:17:32 rk Exp $";
static void *UseId[] = { &UseId, Id };

/** ************************************************************************ **/
/** 				      HEADERS				     **/
/** ************************************************************************ **/

#include "modules_def.h"

/** ************************************************************************ **/
/** 				  LOCAL DATATYPES			     **/
/** ************************************************************************ **/

/** not applicable **/

/** ************************************************************************ **/
/** 				     CONSTANTS				     **/
/** ************************************************************************ **/

/** not applicable **/

/** ************************************************************************ **/
/**				      MACROS				     **/
/** ************************************************************************ **/

/** not applicable **/

/** ************************************************************************ **/
/** 				    LOCAL DATA				     **/
/** ************************************************************************ **/

static	char	module_name[] = "utility.c";	/** File name of this module **/

#if WITH_DEBUGGING_UTIL_2
static	char	_proc_store_hash_value[] = "store_hash_value";
static	char	_proc_clear_hash_value[] = "clear_hash_value";
static	char	_proc_Clear_Global_Hash_Tables[] = "Clear_Global_Hash_Tables";
static	char	_proc_Delete_Global_Hash_Tables[] = "Delete_Global_Hash_Tables";
static	char	_proc_Delete_Hash_Tables[] = "Delete_Hash_Tables";
static	char	_proc_Copy_Hash_Tables[] = "Copy_Hash_Tables";
static	char	_proc_Unwind_Modulefile_Changes[] = "Unwind_Modulefile_Changes";
static	char	_proc_Output_Modulefile_Changes[] = "Output_Modulefile_Changes";
static	char	_proc_Output_Modulefile_Aliases[] = "Output_Modulefile_Aliases";
static	char	_proc_output_set_variable[] = "output_set_variable";
static	char	_proc_output_unset_variable[] = "output_unset_variable";
static	char	_proc_output_function[] = "output_function";
static	char	_proc_output_set_alias[] = "output_set_alias";
static	char	_proc_output_unset_alias[] = "output_unset_alias";
static	char	_proc_getLMFILES[] = "getLMFILES";
static	char	_proc___IsLoaded[] = "__IsLoaded";
static	char	_proc_chk_marked_entry[] = "chk_marked_entry";
static	char	_proc_set_marked_entry[] = "set_marked_entry";
static	char	_proc_get_module_basename[] = "get_module_basename";
static	char	_proc_Update_LoadedList[] = "Update_LoadedList";
static	char	_proc_ForcePath[] = "ForcePath";
static	char	_proc_check_magic[] = "check_magic";
static	char	_proc_cleanse_path[] = "cleanse_path";
static	char	_proc_chop[] = "chop";
#endif
#if WITH_DEBUGGING_UTIL_3
static	char	_proc_set_derelict[] = "set_derelict";
#endif

static	char  cmd_separator = ';';	/** Average command separator	     **/

static	FILE *aliasfile;		/** Temporary file to write aliases  **/
static	char  alias_separator = ';';	/** Alias command separator	     **/

/** ************************************************************************ **/
/**				    PROTOTYPES				     **/
/** ************************************************************************ **/

static	void	 Clear_Global_Hash_Tables( void);
static	int	 Output_Modulefile_Aliases( Tcl_Interp *interp);
static	int	 output_set_variable( const char*, const char*);
static	int	 output_unset_variable( const char* var);
static	void	 output_function( const char*, const char*);
static	int	 output_set_alias( const char*, const char*);
static	int	 output_unset_alias( const char*, const char*);
static	int	 __IsLoaded( Tcl_Interp*, char*, char**, char*, int);
static	char	*get_module_basename( char*);
static	int	 ForcePath( Tcl_Interp*, char*, char*, int);
static	char	*chop( const char*);


/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		store_hash_value				     **
 ** 									     **
 **   Description:	Keeps the old value of the variable around if it is  **
 **			touched in the modulefile to enable undoing a	     **
 **			modulefile by resetting the evironment to it started.**
 ** 									     **
 **			This is the same for unset_shell_variable()	     **
 ** 									     **
 **   First Edition:	92/10/14					     **
 ** 									     **
 **   Parameters:	Tcl_HashTable	*htable		Hash table to be used**
 **			const char	*key		Attached key	     **
 **			const char	*value		Alias value	     **
 ** 									     **
 **   Result:		int	TCL_OK		Successfull completion	     **
 ** 									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int store_hash_value(	Tcl_HashTable* htable,
        		const char*    key,
        		const char*    value)
{
    int   		 new;		/** Return from Tcl_CreateHashEntry  **/
					/** which indicates creation or ref- **/
					/** ference to an existing entry     **/
    char		*tmp;		/** Temp pointer used for disalloc.  **/
    Tcl_HashEntry	*hentry;	/** Hash entry reference	     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_store_hash_value, NULL);
#endif

    /**
     **  Create a hash entry for the key to be stored. If there exists one
     **  so far, its value has to be unlinked.
     **  All values in this hash are pointers to allocated memory areas.
     **/

    hentry = Tcl_CreateHashEntry( htable, (char*) key, &new);
    if( !new) {
	tmp = (char *) Tcl_GetHashValue( hentry);
    	if( tmp)
            free( tmp);
    }

    /**
     **  Set up the new value. strdup allocates!
     **/

    if( value)
        Tcl_SetHashValue( hentry, (char*) strdup((char*) value));
    else
        Tcl_SetHashValue( hentry, (char*) NULL);
    
    return( TCL_OK);

} /** End of 'store_hash_value' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		clear_hash_value				     **
 ** 									     **
 **   Description:	Remove the specified shell variable from the passed  **
 **			hash table					     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_HashTable	*htable		Hash table to be used**
 **			const char	*key		Attached key	     **
 ** 									     **
 **   Result:		int	TCL_OK		Successfull completion	     **
 ** 									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int clear_hash_value(	Tcl_HashTable	*htable,
                       	const char	*key)
{
    char		*tmp;		/** Temp pointer used for disalloc.  **/
    Tcl_HashEntry	*hentry;	/** Hash entry reference	     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_clear_hash_value, NULL);
#endif

    /**
     **  If I haven't already created an entry for keeping this environment
     **  variable's value, then just leave.
     **  Otherwise, remove this entry from the hash table.
     **/

    if( hentry = Tcl_FindHashEntry( htable, (char*) key) ) {

        tmp = (char*) Tcl_GetHashValue( hentry);
        if( tmp)
	    free( tmp);

        Tcl_DeleteHashEntry( hentry);
    }
    
    return( TCL_OK);

} /** End of 'clear_hash_value' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		Clear_Global_Hash_Tables			     **
 ** 									     **
 **   Description: 	Deletes and reinitializes our env. hash tables.	     **
 ** 									     **
 **   First Edition:	92/10/14					     **
 ** 									     **
 **   Parameters:	-						     **
 **   Result:		-						     **
 ** 									     **
 **   Attached Globals:	setenvHashTable,				     **
 **			unsetenvHashTable,				     **
 **			aliasSetHashTable,				     **
 **			aliasUnsetHashTable				     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	void	Clear_Global_Hash_Tables( void)
{
    Tcl_HashSearch	 searchPtr;	/** Tcl hash search handle	     **/
    Tcl_HashEntry	*hashEntry;	/** Result from Tcl hash search      **/
    char		*val = NULL;	/** Stored value (is a pointer!)     **/

    /**
     **  The following hash tables are to be initialized
     **/

    Tcl_HashTable	*table[5],
			**table_ptr = table;

    table[0] = setenvHashTable;
    table[1] = unsetenvHashTable;
    table[2] = aliasSetHashTable;
    table[3] = aliasUnsetHashTable;
    table[4] = NULL;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Clear_Global_Hash_Tables, NULL);
#endif

    /**
     **  Loop for all the hash tables named above. If there's no value stored
     **  in a hash table, skip to the next one. 
     **/

    for( ; *table_ptr; table_ptr++) {

	if( ( hashEntry = Tcl_FirstHashEntry( *table_ptr, &searchPtr)) == NULL) 
	    continue;
	
	/**
	 **  Otherwise remove all values stored in the table
	 **/

	do {
	    val = (char*) Tcl_GetHashValue( hashEntry);
	    if( val)
	        free(val);
	} while( hashEntry = Tcl_NextHashEntry( &searchPtr));

	/**
	 **  Reinitialize the hash table by unlonking it from memory and 
	 **  thereafter initializing it again.
	 **/

	Tcl_DeleteHashTable( *table_ptr);
	Tcl_InitHashTable( *table_ptr, TCL_STRING_KEYS);

    } /** for **/

} /** End of 'Clear_Global_Hash_Tables' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		Delete_Global_Hash_Tables			     **
 **			Delete_Hash_Tables				     **
 ** 									     **
 **   Description: 	Deletes our environment hash tables.		     **
 ** 									     **
 **   First Edition:	92/10/14					     **
 ** 									     **
 **   Parameters:	Tcl_HashTable	**table_ptr	NULL-Terminated list **
 **							of hash tables to be **
 **							deleted		     **
 **   Result:		-						     **
 ** 									     **
 **   Attached Globals:	setenvHashTable,				     **
 **			unsetenvHashTable,				     **
 **			aliasSetHashTable,				     **
 **			aliasUnsetHashTable				     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

void Delete_Global_Hash_Tables( void) {

    /**
     **  The following hash tables are to be initialized
     **/

    Tcl_HashTable	*table[5];

    table[0] = setenvHashTable;
    table[1] = unsetenvHashTable;
    table[2] = aliasSetHashTable;
    table[3] = aliasUnsetHashTable;
    table[4] = NULL;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Delete_Global_Hash_Tables, NULL);
#endif

    Delete_Hash_Tables( table);

} /** End of 'Delete_Global_Hash_Tables' **/

void Delete_Hash_Tables( Tcl_HashTable	**table_ptr)
{

    Tcl_HashSearch	 searchPtr;	/** Tcl hash search handle	     **/
    Tcl_HashEntry	*hashEntry;	/** Result from Tcl hash search      **/
    char		*val = NULL;	/** Stored value (is a pointer!)     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Delete_Hash_Tables, NULL);
#endif

    /**
     **  Loop for all the hash tables named above. Remove all values stored in
     **  the table and then free up the whole table
     **/

    for( ; *table_ptr; table_ptr++) {

        if( ( hashEntry = Tcl_FirstHashEntry( *table_ptr, &searchPtr))) {

	    /**
	     **  Remove all values stored in the table
	     **/

	    do {
		val = (char*) Tcl_GetHashValue( hashEntry);
		if( val)
		    free(val);
	    } while( hashEntry = Tcl_NextHashEntry( &searchPtr));

	    /**
	     **  Remove internal hash control structures
	     **/

	    Tcl_DeleteHashTable( *table_ptr);
	}

        free( (char*) *table_ptr);

    } /** for **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_END, LOC, _proc_Delete_Hash_Tables, NULL);
#endif

} /** End of 'Delete_Hash_Tables' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		Copy_Hash_Tables				     **
 ** 									     **
 **   Description:	Allocate new hash tables for the global environment, **
 **			initialize them and copy the contents of the current **
 **			tables into them.				     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	-						     **
 **   Result:		Tcl_HashTable**		Pointer to the new list of   **
 **						hash tables		     **
 **   Attached Globals:	setenvHashTable,				     **
 **			unsetenvHashTable,				     **
 **			aliasSetHashTable,				     **
 **			aliasUnsetHashTable				     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

Tcl_HashTable	**Copy_Hash_Tables( void)
{
    Tcl_HashSearch	  searchPtr;	/** Tcl hash search handle	     **/
    Tcl_HashEntry	 *oldHashEntry,	/** Hash entries to be copied	     **/
			 *newHashEntry;
    char		 *val = NULL,	/** Stored value (is a pointer!)     **/
    			 *key = NULL;	/** Hash key			     **/
    int			  new;		/** Tcl inidicator, if the new hash  **/
					/** entry has been created or ref.   **/

    Tcl_HashTable	 *oldTable[5],
			**o_ptr, **n_ptr,
			**newTable;	/** Destination hash table	     **/

    oldTable[0] = setenvHashTable;
    oldTable[1] = unsetenvHashTable;
    oldTable[2] = aliasSetHashTable;
    oldTable[3] = aliasUnsetHashTable;
    oldTable[4] = NULL;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Copy_Hash_Tables, NULL);
#endif

    /**
     **  Allocate storage for the new list of hash tables
     **/

    if( !(newTable = (Tcl_HashTable**) malloc( sizeof( oldTable)))) {
	if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL))
	    return( NULL);		/** -------- EXIT (FAILURE) -------> **/
    }

    /**
     **  Now copy each hashtable out of the list
     **/

    for( o_ptr = oldTable, n_ptr = newTable; *o_ptr; o_ptr++, n_ptr++) {

	/**
	 **  Allocate memory for a single hash table
	 **/

	if( !(*n_ptr = (Tcl_HashTable*) malloc( sizeof( Tcl_HashTable)))) {
	    if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL)) {
		free( newTable);
		return( NULL);		/** -------- EXIT (FAILURE) -------> **/
	    }
	}

	/**
	 **  Initialize that guy and copy it from the old table
	 **/

	Tcl_InitHashTable( *n_ptr, TCL_STRING_KEYS);
        if( oldHashEntry = Tcl_FirstHashEntry( *o_ptr, &searchPtr)) {

	    /**
	     **  Copy all entries if there are any
	     **/

	    do {

		key = (char*) Tcl_GetHashKey( *o_ptr, oldHashEntry);
		val = (char*) Tcl_GetHashValue( oldHashEntry);

		newHashEntry = Tcl_CreateHashEntry( *n_ptr, key, &new);

		if(val)
		    Tcl_SetHashValue(newHashEntry, strdup(val));
		else
		    Tcl_SetHashValue(newHashEntry, (char *) NULL);

	    } while( oldHashEntry = Tcl_NextHashEntry( &searchPtr));

	} /** if **/
    } /** for **/

    /**
     **  Put a terminator at the end of the new table
     **/

    *n_ptr = NULL;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_END, LOC, _proc_Copy_Hash_Tables, NULL);
#endif

    return( newTable);

} /** End of 'Copy_Hash_Tables' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:								     **
 ** 									     **
 **   Description:	Once a the loading or unloading of a modulefile	     **
 **			fails, any changes it has made to the environment    **
 **			must be undone and reset to its previous state. This **
 **			function is responsible for unwinding any changes a  **
 **			modulefile has made.				     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp	 *interp	According TCL interp.**
 **			Tcl_HashTable	**oldTables	Hash tables storing  **
 **							the former environm. **
 **   Result:								     **
 **   Attached Globals:							     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int Unwind_Modulefile_Changes(	Tcl_Interp	 *interp, 
				Tcl_HashTable	**oldTables )
{
    Tcl_HashSearch	 searchPtr;	/** Tcl hash search handle	     **/
    Tcl_HashEntry	*hashEntry;	/** Result from Tcl hash search      **/
    char		*val = NULL,	/** Stored value (is a pointer!)     **/
			*key;		/** Tcl hash key		     **/
    int			 i;		/** Loop counter		     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Unwind_Modulefile_Changes, NULL);
#endif

    if( oldTables) {

	/**
	 **  Use only entries 0 and 1 which do contain all changes to the 
	 **  shell varibles (setenv and unsetenv)
	 **/

	/** ??? What about the aliases (table 2 and 3) ??? **/

	for( i = 0; i < 2; i++) {
	    if( hashEntry = Tcl_FirstHashEntry( oldTables[i], &searchPtr)) {

		do {
		    key = (char*) Tcl_GetHashKey( oldTables[i], hashEntry);

		    /**
		     **  The hashEntry will contain the appropriate value for the
		     **  specified 'key' because it will have been aquired depending
		     **  upon whether the unset or set table was used.
		     **/

		    val = (char*) Tcl_GetHashValue( hashEntry);
		    if( val)
			Tcl_SetVar2( interp, "env", key, val, TCL_GLOBAL_ONLY);

		} while( hashEntry = Tcl_NextHashEntry( &searchPtr) );

	    } /** if **/
	} /** for **/

	/**
	 **  Delete and reset the hash tables now that the current contents have been
	 **  flushed.
	 **/

	Delete_Global_Hash_Tables();

	setenvHashTable     = oldTables[0];
	unsetenvHashTable   = oldTables[1];
	aliasSetHashTable   = oldTables[2];
	aliasUnsetHashTable = oldTables[3];

    } else {

	Clear_Global_Hash_Tables();

    }

    return( TCL_OK);

} /** End of 'Unwind_Modulefile_Changes' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		Output_Modulefile_Changes			     **
 ** 									     **
 **   Description:	Is used to flush out the changes of the current	     **
 **			modulefile in a manner depending upon whether the    **
 **			modulefile was successfull or unsuccessfull.	     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp	*interp		The attached Tcl in- **
 **							terpreter	     **
 ** 									     **
 **   Result:		int	TCL_OK		Successful operation	     **
 ** 									     **
 **   Attached Globals:	setenvHashTable,				     **
 **			unsetenvHashTable,				     **
 **			aliasSetHashTable,	via Output_Modulefile_Aliases**
 **			aliasUnsetHashTable	via Output_Modulefile_Aliases**
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int Output_Modulefile_Changes(	Tcl_Interp	*interp)
{
    Tcl_HashSearch	 searchPtr;	/** Tcl hash search handle	     **/
    Tcl_HashEntry	*hashEntry;	/** Result from Tcl hash search      **/
    char		*val = NULL,	/** Stored value (is a pointer!)     **/
			*key;		/** Tcl hash key		     **/
    int			 i;		/** Loop counter		     **/

    /**
     **  The following hash tables do contain all changes to be made on
     **  shell variables
     **/

    Tcl_HashTable	*table[2];

    table[0] = setenvHashTable;
    table[1] = unsetenvHashTable;
  
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Output_Modulefile_Changes, NULL);
#endif

    aliasfile = stdout;

    /**
     **  Scan both table that are of interest for shell variables
     **/

    for(i = 0; i < 2; i++) {
	if( hashEntry = Tcl_FirstHashEntry( table[i], &searchPtr)) {

	    do {
		key = (char*) Tcl_GetHashKey( table[i], hashEntry);

		/**
		 **  The table list indicator is used in order to differ
		 **  between the setenv and unsetenv operation
		 **/

		if( i == 1) {
		    output_unset_variable( (char*) key);
		} else {
		    if( val = Tcl_GetVar2( interp, "env", key, TCL_GLOBAL_ONLY)) 
			output_set_variable( (char*) key, val);
		}

	    } while( hashEntry = Tcl_NextHashEntry( &searchPtr));

	} /** if **/
    } /** for **/

    if( EOF == fflush( stdout))
	if( OK != ErrorLogger( ERR_FLUSH, LOC, _fil_stdout, NULL))
	    return( TCL_ERROR);		/** -------- EXIT (FAILURE) -------> **/

    Output_Modulefile_Aliases( interp);
  
    /**
     **  Delete and reset the hash tables now that the current contents have been
     **  flushed.
     **/

    Clear_Global_Hash_Tables();
    return( TCL_OK);

} /* End of 'Output_Modulefile_Changes' */ 

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		Output_Modulefile_Aliases			     **
 ** 									     **
 **   Description:	Is used to flush out the changes to the aliases of   **
 **			the current modulefile. But, some shells don't work  **
 **			well with having their alias information set via the **
 **			'eval' command.  So, what we'll do now is output the **
 **			aliases into a /tmp dotfile, have the shell source   **
 **			the /tmp dotfile and then have the shell remove the  **
 **			/tmp dotfile.					     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp	*interp		The attached Tcl in- **
 **							terpreter	     **
 ** 									     **
 **   Result:		int	TCL_OK		Successful operation	     **
 ** 									     **
 **   Attached Globals: aliasSetHashTable,	via Output_Modulefile_Aliases**
 **			aliasUnsetHashTable	via Output_Modulefile_Aliases**
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	int Output_Modulefile_Aliases( Tcl_Interp *interp)
{
    Tcl_HashSearch	 searchPtr;	/** Tcl hash search handle	     **/
    Tcl_HashEntry	*hashEntry;	/** Result from Tcl hash search      **/
    char		*val = NULL,	/** Stored value (is a pointer!)     **/
			*key;		/** Tcl hash key		     **/
    int			 i;		/** Loop counter		     **/
    char		*sourceCommand; /** Command used to source the alias **/

    /**
     **  The following hash tables do contain all changes to be made on
     **  shell aliases
     **/

    Tcl_HashTable	*table[2];

#ifndef EVAL_ALIAS

    /**
     **  If configured so, all changes to aliases are written into a temporary
     **  file which is sourced by the invoking shell ...
     **  In this case a temporary filename has to be assigned for the alias
     **  source file. The file has to be openend as 'aliasfile'.
     **  The default for aliasfile, if no shell sourcing is used, is stdout.
     **/

#ifdef HAVE_TEMPNAM
    char*            aliasfilename = (char *)tempnam(NULL, "M_od_");
#else
#ifdef HAVE_TMPNAM
    char*            aliasfilename[L_tmpnam + 16]; /* Just to be sure... */
    tmpnam((char *)aliasfilename);
#else /* not HAVE_TMPNAM */
    char*            aliasfilename = "M_od_temp";
#endif /* not HAVE_TMPNAM */
#endif /* not HAVE_TEMPNAM */
#endif /* not EVAL_ALIAS */

    table[0] = aliasSetHashTable;
    table[1] = aliasUnsetHashTable;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Output_Modulefile_Aliases, NULL);
#endif

#ifndef EVAL_ALIAS
    /**
     **  We only need to output stuff into a temporary file if we're setting
     **  stuff.  We can unset variables and aliases by just using eval.
     **/

    if( hashEntry = Tcl_FirstHashEntry( aliasSetHashTable, &searchPtr)) {
	/**
	 **  We only support sh and csh varients for aliases.  If not either
	 **  sh or csh print warning message and return
	 **/

	if( !strcmp( shell_derelict, "csh")) {
	    sourceCommand = "source %s%c";
	} else if( !strcmp( shell_derelict, "sh")) {
	    sourceCommand = ". %s%c";
	} else {
	    return( TCL_ERROR);	/** -------- EXIT (FAILURE) -------> **/
	}

	/**
	 **  Open the file ...
	 **/

	if( !( aliasfile = fopen((char *) aliasfilename, "w+"))) {
	    if( OK != ErrorLogger( ERR_OPEN, LOC, aliasfilename, "append", NULL))
		return( TCL_ERROR);	/** -------- EXIT (FAILURE) -------> **/

	} else {

	    /**
	     **  Only the source command has to be flushed to stdout. After
	     **  sourcing the alias definition (temporary) file, the source
	     **  file is to be removed.
	     **/

	    alias_separator = '\n';

	    fprintf( stdout, sourceCommand, aliasfilename, cmd_separator);
		fprintf( stdout, "/bin/rm -f %s%c", aliasfilename, cmd_separator);
	} /** if( fopen) **/
    } /** if( alias to set) **/

    free( aliasfilename);

#endif /* EVAL_ALIAS */
  
    /**
     **  Scan the hash tables involved in changing aliases
     **/

    for( i=0; i<2; i++) {
    
	if( hashEntry = Tcl_FirstHashEntry( table[i], &searchPtr)) {

	    do {
		key = (char*) Tcl_GetHashKey( table[i], hashEntry);
		val = (char*) Tcl_GetHashValue( hashEntry);

		/**
		 **  The hashtable list index is used to differ between aliases
		 **  to be set and aliases to be reset
		 **/

		if(i == 1) {
		    output_unset_alias( key, val);
		} else {
		    output_set_alias( key, val);
		}

	    } while( hashEntry = Tcl_NextHashEntry( &searchPtr));

	} /** if **/
    } /** for **/

#ifndef EVAL_ALIAS
    if( EOF == fclose( aliasfile))
	if( OK != ErrorLogger( ERR_CLOSE, LOC, aliasfile, NULL))
	    return( TCL_ERROR);		/** -------- EXIT (FAILURE) -------> **/
#endif

    return( TCL_OK);

} /** End of 'Output_Modulefile_Aliases' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		output_set_variable				     **
 ** 									     **
 **   Description:	Outputs the command required to set a shell variable **
 **			according to the current shell			     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char	*var	Name of the variable to be   **
 **						set			     **
 **			const char	*val	Value to be assigned	     **
 **									     **
 **   Result:		int	TCL_OK		Finished successfull	     **
 **				TCL_ERROR	Unknown shell type	     **
 **									     **
 **   Attached Globals:	shell_derelict					     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	int	output_set_variable(	const char	*var,
          	          		const char	*val)
{

    /**
     **  Differ between the different kinds od shells at first
     **
     **  CSH
     **/

    chop( val);
    chop( var);

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_output_set_variable, " var='", var,
	"' val= '", val, "'", NULL);
#endif

    if( !strcmp((char*) shell_derelict, "csh")) {

#ifdef LMSPLIT_SIZE

	/**
	 **  Many C Shells (specifically the Sun one) has a hard limit on
	 **  the size of the environment variables around 1k.  The
	 **  _LMFILES_ variable can grow beyond 1000 characters.  So, I'm
	 **  going to break it up here since I can put it back together
	 **  again when I use it.
	 **
	 **  You can set the split size using --with-split-size=<number>
	 **  it should probably be <1000.  I don't count the size of
	 **  "setenv _LMFILES_xxx" so subtract this from your limit.
	 **/

	int	lmfiles_len;

	if( !strcmp( var, "_LMFILES_") &&
	    (lmfiles_len = strlen(val)) > LMSPLIT_SIZE) {

	    char buffer[ LMSPLIT_SIZE + 1];
	    int count = 0;

	    /**
	     **  Break up the _LMFILES_ variable...
	     **/

	    while( lmfiles_len > LMSPLIT_SIZE) {

		strncpy( buffer, ( val + count*LMSPLIT_SIZE ), LMSPLIT_SIZE);
		buffer[ LMSPLIT_SIZE] = '\0';

		fprintf( stdout, "setenv %s%03d %s%c", var, count, buffer,
		    cmd_separator);

		lmfiles_len -= LMSPLIT_SIZE;
		count++;
	    }

	    if( lmfiles_len)
		fprintf( stdout, "setenv %s%03d %s%c", var, count,
		    (val + count*LMSPLIT_SIZE), cmd_separator);

	    /**
	     **  Unset _LMFILES_ as indicator to use the multi-variable _LMFILES_
	     **/

	    fprintf(stdout, "unsetenv %s%c", var, cmd_separator);

	} else {	/** if( var == "_LMFILES_" **/

	    fprintf(stdout, "setenv %s %s%c", var, val, cmd_separator);
	}

#else /* not LMSPLIT_SIZE */

      fprintf(stdout, "setenv %s %s%c", var, val, cmd_separator);

#endif

    /**
     **  SH
     **/

    } else if( !strcmp((char*) shell_derelict, "sh")) {
	fprintf( stdout, "%s=%s%cexport %s%c", var, val, cmd_separator,
	    var, cmd_separator);

    /**
     **  EMACS
     **/

    } else if( !strcmp((char*) shell_derelict, "emacs")) {
	fprintf( stdout, "(setenv \"%s\" \"%s\")\n", var, val);

    /**
     **  PERL
     **/

    } else if( !strcmp((char*) shell_derelict, "perl")) {
	fprintf( stdout, "$ENV{'%s'} = \"%s\"%c", var, val, cmd_separator);  

    /**
     **  PYTHON
     **/

    } else if( !strcmp((char*) shell_derelict, "python")) {
	fprintf( stdout, "os.environ['%s'] = '%s'\n", var, val);

    /**
     **  Unknown shell type - print an error message and 
     **  return on error
     **/

    } else {
	if( OK != ErrorLogger( ERR_DERELICT, LOC, shell_derelict, NULL))
	    return( TCL_ERROR);		/** -------- EXIT (FAILURE) -------> **/
    }

    /**
     **  Return and acknowldge success
     **/

    return( TCL_ERROR);

} /** End of 'output_set_variable' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		output_unset_variable				     **
 ** 									     **
 **   Description:	Outputs the command required to unset a shell        **
 **			variable according to the current shell		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char	*var	Name of the variable to be   **
 **						unset			     **
 **									     **
 **   Result:		int	TCL_OK		Finished successfull	     **
 **				TCL_ERROR	Unknown shell type	     **
 **									     **
 **   Attached Globals:	shell_derelict					     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	int	output_unset_variable( const char* var)
{
    chop( var);

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_output_unset_variable, NULL);
#endif

    /**
     **  Display the 'unsetenv' command according to the current invoking shell.
     **/

    if( !strcmp( shell_derelict, "csh")) {
	fprintf( stdout, "unsetenv %s%c", var, cmd_separator);
    } else if( !strcmp( shell_derelict, "sh")) {
	fprintf( stdout, "unset %s%c", var, cmd_separator);
    } else if( !strcmp( shell_derelict, "emacs")) {
	fprintf( stdout, "(setenv \"%s\" nil)\n", var);
    } else if( !strcmp( shell_derelict, "perl")) {
	fprintf( stdout, "delete $ENV{'%s'}%c", var, cmd_separator);  
    } else if( !strcmp( shell_derelict, "python")) {
      fprintf( stdout, "os.environ['%s'] = ''\ndel os.environ['%s']\n",
	       var, var);
    } else {
	if( OK != ErrorLogger( ERR_DERELICT, LOC, shell_derelict, NULL))
	    return( TCL_ERROR);		/** -------- EXIT (FAILURE) -------> **/
    }

    /**
     **  Return and acknowldge success
     **/

    return( TCL_OK);

} /** End of 'output_unset_variable' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		set_derelict					     **
 ** 									     **
 **   Description:	Normalize the current calling shell to one of the    **
 **			basic shells definig the varaible and alias syntax   **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char	*name	Invoking shell name	     **
 ** 									     **
 **   Result:		char*			Shell derelict name	     **
 ** 									     **
 **   Attached Globals:	shell_derelict					     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

char	*set_derelict(	const char	*name) 
{

#if WITH_DEBUGGING_UTIL_3
    ErrorLogger( NO_ERR_START, LOC, _proc_set_derelict, NULL);
#endif

    /**
     **  Use bourne shell syntax for SH, BASH, ZSH and KSH
     **/

    if( !strcmp((char*) name, "sh") || 
        !strcmp((char*) name, "bash") || 
        !strcmp((char*) name, "zsh") || 
        !strcmp((char*) name, "ksh")) {
	return( strcpy( shell_derelict, "sh"));

    /**
     **  CSH and TCSH
     **/

    } else if( !strcmp((char*) name, "csh") || 
	       !strcmp((char*) name, "tcsh")) {
	return( strcpy( shell_derelict, "csh"));

    /** 
     **  EMACS
     **/ 

    } else if( !strcmp((char*) name, "emacs")) {
	return( strcpy( shell_derelict, "emacs"));

    /** 
     **  PERL
     **/ 

    } else if( !strcmp((char*) name, "perl")) {
	return( strcpy( shell_derelict, "perl"));

    /**
     ** PYTHON
     **/

    } else if( !strcmp((char*) name, "python")) {
	return( strcpy( shell_derelict, "python"));
    }

    /**
     **  Oops! Undefined shell name ...
     **/

    return( NULL);

} /** End of 'set_derelict' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		output_function					     **
 ** 									     **
 **   Description:	Actually turns the Modules set-alias information     **
 **			into a string that a shell can source.  Previously,  **
 **			this routine just output the alias information to be **
 **			eval'd by the shell.				     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char	*var	Name of the alias to be set  **
 **			const char	*val	Value to be assigned	     **
 ** 									     **
 **   Result:		-						     **
 ** 									     **
 **   Attached Globals:	aliasfile,	The output file for alias commands.  **
 **					see 'Output_Modulefile_Aliases'      **
 **			alias_separator					     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	void	output_function(	const char	*var,
					const char	*val)
{
    const char	*cptr = val;
    int 	nobackslash = 1;
    
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_output_function, NULL);
#endif

    /**
     **  This opens a function ...
     **/

    fprintf( aliasfile, "%s() {%c", var, alias_separator);

    /**
     **  ... now print the value. Print it as a single line and remove any
     **  backslash
     **/

    while( *cptr) {

        if( *cptr == '\\') {
            if( !nobackslash)
		putc( *cptr, aliasfile);
            else
		nobackslash = 0;
            cptr++;
            continue;
        } else
            nobackslash = 1;

        putc(*cptr++, aliasfile);

    } /** while **/

    /**
     **  Finally close the function
     **/

    fprintf( aliasfile, ";%c}%c", alias_separator,alias_separator);

} /** End of 'output_function' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		output_set_alias				     **
 ** 									     **
 **   Description:	Flush the commands required to set shell aliases de- **
 **			pending on the current invoking shell		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char	*alias		Name of the alias    **
 **			const char	*val		Value to be assigned **
 ** 									     **
 **   Result:		int	TCL_OK		Operation successfull	     **
 ** 									     **
 **   Attached Globals:	aliasfile, 	The alias command is writte out to   **
 **			alias_separator Defined the command separator	     **
 **			shell_derelict	to determine the shell family	     **
 **			shell_name	to determine the real shell type     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	int	output_set_alias(	const char	*alias,
               			  	const char	*val)
{
    int nobackslash = 1;		/** Controls wether backslashes are  **/
					/** to be print			     **/
    const char *cptr = val;		/** Scan the value char by char	     **/
        
    
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_output_set_alias, NULL);
#endif

    /**
     **  Check fot the shell family
     **  CSHs need to switch $* to \!* and $n to \!\!:n unless the $ has a
     **  backslash before it
     **/

    if( !strcmp( shell_derelict, "csh")) {

	/**
	 **  On CSHs the command is 'alias <name> <value>'. Print the beginning
	 **  of the command and then print the value char by char.
	 **/

        fprintf( aliasfile, "alias %s '", alias);

        while( *cptr) {

	    /**
	     **  Convert $n to \!\!:n
	     **/

            if( *cptr == '$' && nobackslash) {
                cptr++;
                if( *cptr == '*')
                    fprintf( aliasfile, "\\!");
                else
                    fprintf( aliasfile, "\\!\\!:");
            }

	    /**
	     **  Recognize backslashes
	     **/

            if( *cptr == '\\') {
                if( !nobackslash)
		    putc( *cptr, aliasfile);
                else
		    nobackslash = 0;
                cptr++;
                continue;
            } else
                nobackslash = 1;

	    /**
	     **  print the read character
	     **/

            putc( *cptr++, aliasfile);

        } /** while **/
 
	/**
	 **  Now close up the command using the alias command terinator as
	 **  defined in the according global variable
	 **/

        fprintf( aliasfile, "'%c", alias_separator);

    /**
     **  Bourne shell family: The alias has to be  translated into a
     **  function using the function call 'output_function'
     **/

    } else if( !strcmp(shell_derelict, "sh")) {

	/**
	 **  The bourne shell itsself
         **  need to write a function unless this sh doesn't support
	 **  functions
	 **/

        if( !strcmp( shell_name, "sh")) {
#ifdef HAS_BOURNE_FUNCS
            output_function(alias, val);
#else
	    /** ??? Print an error message ??? **/
#endif

	/**
	 **  Shells supportig extended bourne shell syntax ....
	 **/

        } else if( !strcmp( shell_name, "bash") ||
                   !strcmp( shell_name, "zsh" ) ||
                   !strcmp( shell_name, "ksh")) {

	    /**
	     **  in this case we only have to write a function if the alias
	     **  take arguments. This is the case if the value has somewhere
	     **  a '$' in it.
	     **/

            while( *cptr && *cptr != '$')
		cptr++;

	    if( *cptr == '$') {
		output_function( alias, val);
		return TCL_OK;
	    } else

            /**
             **  So, we can just output an alias...
             **/

		fprintf( aliasfile, "alias %s='%s'%c", alias, val,
		    alias_separator);

        } /** if( bash, zsh, ksh) **/

	/** ??? Unknwn derelict ??? **/

    } /** if( !csh ) **/

    return( TCL_OK);

} /** End of 'output_set_alias' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		output_unset_alias				     **
 ** 									     **
 **   Description:	Flush the commands required to reset shell aliases   **
 **			depending on the current invoking shell		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char	*alias		Name of the alias    **
 **			const char	*val		Value which has been **
 **							assigned	     **
 ** 									     **
 **   Result:		int	TCL_OK		Operation successfull	     **
 ** 									     **
 **   Attached Globals:	aliasfile, 	The alias command is writte out to   **
 **			alias_separator Defined the command separator	     **
 **			shell_derelict	to determine the shell family	     **
 **			shell_name	to determine the real shell type     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	int	output_unset_alias(	const char	*alias,
					const char	*val)
{
    const char *cptr = val;	/** Need to read the value char by char      **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_output_unset_alias, NULL);
#endif

    /**
     **  Check for the shell family at first
     **  Ahh! CSHs ... ;-)
     **/

    if( !strcmp( shell_derelict, "csh")) {
        fprintf( aliasfile, "unalias %s%c", alias, alias_separator);

    /**
     **  Hmmm ... bourne shell types ;-(
     **  Need to unset a function in case of sh or if the alias took parameters
     **/

    } else if( !strcmp( shell_derelict, "sh")) {

        if( !strcmp( shell_name, "sh")) {
            fprintf( aliasfile, "unset -f %s%c", alias, alias_separator);

	/**
	 **  BASH
	 **/

        } else if( !strcmp( shell_name, "bash")) {

            /**
             **  If we have what the old value should have been, then look to
             **  see if it was a function or an alias because bash spits out an
             **  error if you try to unalias a non-existent alias.
             **/

            if(val) {

                /**
                 **  Was it a function?
                 **  Yes, if it has arguments...
                 **/

                while( *cptr && *cptr != '$')
                    cptr++;

		if(*cptr == '$') {
		    fprintf(aliasfile, "unset -f %s%c", alias, alias_separator);
		    return TCL_OK;
		} else

                /**
                 **  Well, it wasn't a function, so we'll put out an unalias...
                 **/

		    fprintf( aliasfile, "unalias %s%c", alias, alias_separator);

            } else {	/** No value known (any more?) **/

                /**
                 **  We'll assume it was a function because the unalias command
                 **  in bash produces an error.  It's possible that the alias
                 **  will not be cleared properly here because it was an
                 **  unset-alias command.
                 **/

                fprintf( aliasfile, "unset -f %s%c", alias, alias_separator);
            }

	/**
	 **  ZSH or KSH
	 **  Put out both because we it could be either a function or an
	 **  alias.  This will catch both.
	 **/

        } else if( !strcmp( shell_name, "zsh") || !strcmp( shell_name, "ksh")) {

            fprintf(aliasfile, "unalias %s%c", alias, alias_separator);
            fprintf(aliasfile, "unset -f %s%c", alias, alias_separator);

        } /** if( bash, zsh, ksh) **/

	/** ??? Unknown derelict ??? **/

    } /** if( sh-family) **/
    
    return( TCL_OK);

} /** End of 'output_unset_alias' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		getLMFILES					     **
 ** 									     **
 **   Description:	Read in the _LMFILES_ environment variable. This one **
 **			may be split into several variables cause by limited **
 **			variable space of some shells (esp. the SUN csh)     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp    *interp	Attached Tcl interpreter     **
 ** 									     **
 **   Result:		char*	Value of the environment varibale _LMFILES_  **
 ** 									     **
 **   Attached Globals:							     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

char	*getLMFILES( Tcl_Interp	*interp)
{
    static char	*lmfiles = NULL;	/** Buffer pointer for the value     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_getLMFILES, NULL);
#endif

    /**
     **  Try to read the variable _LMFILES_. If the according buffer pointer
     **  contains a value, disallocate it before.
     **/

    if( lmfiles)
        free(lmfiles);

    lmfiles = Tcl_GetVar2( interp, "env", "_LMFILES_", TCL_GLOBAL_ONLY);

    /**
     **  Now the pointer is NULL in case of the variable has not been defined.
     **  In this case try to read in the splitted variable from _LMFILES_xxx
     **/

    if( !lmfiles) {

        char	buffer[ MOD_BUFSIZE];	/** Used to set up the split variab- **/
					/** les name			     **/
        int	count = 0;		/** Split part count		     **/
        int	lmsize = 0;		/** Total size of _LMFILES_ content  **/
        int	old_lmsize;		/** Size save buffer		     **/
        int	cptr_len;		/** Size of the current split part   **/
        char	*cptr;			/** Split part read pointer	     **/

	/**
	 **  Set up the split part environment variable name and try to read it
	 **  in
	 **/

        sprintf( buffer, "_LMFILES_%03d", count++);
        cptr = Tcl_GetVar2( interp, "env", buffer, TCL_GLOBAL_ONLY);

        while( cptr) {			/** Something available		     **/

	    /**
	     **  Count up the variables length
	     **/

            cptr_len = strlen( cptr);	
            old_lmsize = lmsize;
            lmsize += cptr_len;
 
	    /**
	     **  Reallocate the value's buffer and copy the current split
	     **  part at its end
	     **/

            if((char *) NULL == (lmfiles =
		(char*) realloc( lmfiles, lmsize * sizeof(char) + 1))) {
		    if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL))
			return( NULL);		/** ---- EXIT (FAILURE) ---> **/
	    }
            
            strncpy( lmfiles + old_lmsize, cptr, cptr_len);
            *(lmfiles + old_lmsize + cptr_len) = '\0';
 
	    /**
	     **  Read the next split part variable
	     **/

            sprintf( buffer, "_LMFILES_%03d", count++);
            cptr = Tcl_GetVar2( interp, "env", buffer, TCL_GLOBAL_ONLY);

        }

    } else {  /** if( lmfiles) **/

	/**
	 **  If the environvariable _LMFILES_ has been set, copy the contents
	 **  of the returned buffer into a free allocated one in order to
	 **  avoid side effects.
	 **/

	char	*tmp = strdup(lmfiles);

	if( !tmp)
	    if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL))
		return( NULL);		/** -------- EXIT (FAILURE) -------> **/

	/** 
	 **  Set up lmfiles pointing to the new buffer in order to be able to
	 **  disallocate when invoked next time.
	 **/

        lmfiles = tmp;

    } /** if( lmfiles) **/

    /**
     **  Return the received value to the caller
     **/

    return( lmfiles);

} /** end of 'getLMFILES' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		IsLoaded					     **
 ** 									     **
 **   Description:	Check wether the passed modulefile is cirrently      **
 **			loaded						     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp       *interp	According Tcl interp.**
 **			char             *modulename	Name of the module to**
 **							be searched for	     **
 **			char            **realname	Buffer for the name  **
 **							and version of the   **
 **							module that has mat- **
 **							ched the query	     **
 **			char             *filename	Buffer to store the  **
 **							whole filename of a  **
 **							found loaded module  **
 **									     **
 **   Result:		int	0	Requested module not loaded	     **
 **				1	module is loaded		     **
 **									     **
 **			realname	points to the name of the module that**
 **					has matched the query. If this poin- **
 **					differs form 'modulename' after this **
 **					function has finished, the buffer for**
 **					to store the module name in has been **
 **					allocated here.			     **
 **					if (char **) NULL is passed, no buf- **
 **					fer will be allocated		     **
 **				??? Is this freed correctly by the caller ???**
 **									     **
 **			filename	will be filled with the full module  **
 **					file path of the module that has     **
 **					matched the query		     **
 **									     **
 **   Attached Globals:							     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

/**
 **  Check all possibilities of module-versions
 **/

int IsLoaded(	Tcl_Interp	 *interp,
		char		 *modulename,
		char		**realname,
		char		 *filename )
{
    return( __IsLoaded( interp, modulename, realname, filename, 0));
}

/**
 **  Check only an exact match of the passed module and version
 **/

int IsLoaded_ExactMatch(	Tcl_Interp	 *interp,
				char		 *modulename,
				char		**realname,
				char		 *filename )
{
    return( __IsLoaded( interp, modulename, realname, filename, 1));
}

/**
 **  The subroutine __IsLoaded finally checks for the requested module being
 **  loaded or not.
 **/

static int __IsLoaded(	Tcl_Interp	 *interp,
			char		 *modulename,
			char		**realname,
			char		 *filename,
			int		  exact)
{
    char *l_modules = NULL;		/** Internal module list buffer	     **/
    char *l_modulefiles = NULL;		/** Internal module file list buffer **/
    char *loaded = NULL;		/** Buffer for the module            **/
    char *basename = NULL;		/** Pointer to module basename       **/
    char *loadedmodule_path = NULL;	/** Pointer to one loaded module out **/
					/** of the loaded modules list	     **/
    int   count = 0;

    /**
     **  Get a list of loaded modules (environment variable 'LOADEDMODULES')
     **  and the list of loaded module-files (env. var. __LMFILES__)
     **/

    char	*loaded_modules = Tcl_GetVar2( interp, "env", "LOADEDMODULES",
				    TCL_GLOBAL_ONLY);
    char	*loaded_modulefiles = getLMFILES( interp);
    
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc___IsLoaded, NULL);
#endif

    /**
     **  If no module is currently loaded ... the requested module is shurely
     **  not loaded, too ;-)
     **/

    if( !loaded_modules) 
	return( 0);			/** -------- EXIT PROCEDURE -------> **/
    
    /**
     **  Copy the list of currently loaded modules into a new allocated array
     **  for further handling. If this failes it will be assumed, that the 
     **  module is *NOT* loaded.
     **/

    l_modules = strdup(loaded_modules);
    if( !l_modules) {
	if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL))
	    return( 0);			/** -------- EXIT (FAILURE) -------> **/
    }

    /**
     **  Copy the list of currently loaded modulefiles into a new allocated
     **  array for further handling. If this failes it will be assumed, that
     **  the module is *NOT* loaded.
     **/

    if(loaded_modulefiles) {
	l_modulefiles = strdup( loaded_modulefiles);
	if( !l_modulefiles) {
	    if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL))
		return( 0);		/** -------- EXIT (FAILURE) -------> **/
        }
    }

    /**
     **  Assume the modulename given was an exact match so there is no
     **  difference to return -- this will change in the case it wasn't an
     **  exact match below
     **/

    if( realname)
        *realname = modulename;

    if( *l_modules) {

	/**
	 **  Get each single module which is loaded by splitting up at colons
	 **  The variable LOADEDMODULES contains a list of modulefile like the
	 **  following:
	 **                gnu/2.0:openwin/3.0
	 **/

	loadedmodule_path = strtok( l_modules, ":");
	while( loadedmodule_path) {

	    loaded = strdup( loadedmodule_path);
	    if ( !loaded) {
		if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL)) {
		    if( l_modulefiles)
			free( l_modulefiles);
		    free ( l_modules);
		    return( 0);		/** -------- EXIT PROCEDURE -------> **/
		}
	    }

	    /**
	     **  Get a modulefile without a version and check if this is the
	     **  requested one.
	     **/

	    if( !strcmp( loaded, modulename)) {	/** FOUND    **/

		free ( loaded);
		break;			/** leave the while loop	     **/

	    } else if( !exact) {		/** NOT FOUND	     **/

		/**
		 **  Try to more and more simplify the modulename by removing
		 **  all detail (version) information
		 **/

		basename = get_module_basename( loaded);
		while( basename && strcmp( basename, modulename)) {
		    basename = get_module_basename( basename);
                }

		/**
		 **  Something left after splitting again? If yes the requested
		 **  module is found!
                 **  Since the name given was a basename, return the fully
		 **  loaded path
		 **/

                if( basename) {

		    free( loaded);

                    if( realname) {
			*realname = strdup( loaded);
			if ( !*realname) {
			    if( OK != ErrorLogger( ERR_ALLOC, LOC, NULL)) {
				if( l_modulefiles)
				    free( l_modulefiles);
				free ( l_modules);
				return( 0);	/** ---- EXIT PROCEDURE ---> **/
			    }
                        }
                    }

		    break;		/** leave the while loop	     **/

		} /** if( loaded) **/
	    } /** if not found with single basename **/

	    /**
	     **  Get the next entry from the loaded modules list
	     **/

	    loadedmodule_path = strtok( NULL, ":");
            count++;

	    free( loaded);		/** Free what has been alloc. **/

	} /** while **/
    } /** if( *l_modules) **/

    /**
     **  If we found something locate it's associated modulefile
     **/

    if( loadedmodule_path) {
        if( filename && l_modulefiles && *l_modulefiles) {

	    /**
	     **  The position of the loaded module within the list of loaded
	     **  modules has been counted in 'count'. The position of the 
	     **  associated modulefile should be the same. So tokenize the
	     **  list of modulefiles by the colon until the wanted position
	     **  is reached.
	     **/

            char* modulefile_path = strtok(l_modulefiles, ":");
	
            while( count) {
                if( !( modulefile_path = strtok( NULL, ":"))) {

		    /**
		     **  Oops! Fewer entries in the list of loaded modulefiles
		     **  than in the list of loaded modules. This will
		     **  generally suggest that _LMFILES_ has become corrupted,
		     **  but it may just mean we're working intermittantly with
		     **  an old version.  So, I'll just not touch filename which
		     **  means the search will continue using the old method of
		     **  looking through MODULEPATH.  
                     */

                    free( l_modulefiles);
                    free( l_modules);
                    return( 1);		/** -------- EXIT PROCEDURE -------> **/
                }
                count--;

            } /** while **/

	    /**
	     **  Copy the result into the buffer passed from the caller
	     **/

            strcpy( filename, modulefile_path);
        }

	/**
	 **  FOUND.
	 **  free up everything which has been allocetd and return on success
	 **/

	if( l_modulefiles)
	    free( l_modulefiles);
	free( l_modules);
	return( 1);			/** -------- EXIT PROCEDURE -------> **/
    }

    /**
     **  NOT FOUND. Free up everything which has been alloc'd and return on
     **  failure
     **/

    free( l_modules);
    if( l_modulefiles)
    free( l_modulefiles);

    return( 0);

} /** End of '__IsLoaded' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		chk_marked_entry, set_marked_entry		     **
 ** 									     **
 **   Description:	When switching, the variables are marked with a mar- **
 **			ker that is tested to see if the variable was changed**
 **			in the second modulefile. If it was not, then the    **
 **			variable is unset.				     **
 ** 									     **
 **   First Edition:	92/10/25					     **
 ** 									     **
 **   Parameters:	Tcl_HashTable   *table	Attached hash table	     **
 **			char            *var	According variable name	     **
 **			int              val	Value to be set.	     **
 **									     **
 **   Result:		int	0	Mark not set (or the value of the    **
 **					mark was 0 ;-)			     **
 **				else	Value of the mark that has been set  **
 **					with set_marked_entry.		     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int chk_marked_entry(	Tcl_HashTable	*table,
			char		*var)
{
    Tcl_HashEntry 	*hentry;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_chk_marked_entry, NULL);
#endif

    if( hentry = Tcl_FindHashEntry( table, var))
        return((int) Tcl_GetHashValue( hentry));
    else
        return 0;
}

void set_marked_entry(	Tcl_HashTable	*table,
			char		*var,
			int 		 val)
{
    Tcl_HashEntry	*hentry;
    int    		 new;

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_set_marked_entry, NULL);
#endif

    if( hentry = Tcl_CreateHashEntry( table, var, &new)) {
        if( val)
            Tcl_SetHashValue( hentry, val);
    }

    /**  ??? Shouldn't there be an error return in case of hash creation
	     failing ??? **/
}

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		get_module_basename				     **
 ** 									     **
 **   Description:	Get the name of a module without its version.	     **
 **			This function modifies the string passed in.	     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	char	*modulename		Full module name     **
 ** 									     **
 **   Result:		char*		Module name without version	     **
 ** 									     **
 **   Attached Globals:							     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	char	*get_module_basename(	char	*modulename)
{
    char *version;			/** Used to locate the version sep.  **/
    
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_get_module_basename, NULL);
#endif

    /**
     **  Use strrchr to locate the very last version string on the module
     **  name.
     **/

    if((version = strrchr( modulename, '/'))) {
	*version = '\0';
    } else {
	modulename = NULL;
    }
 
    /**
     **  Return the *COPIED* string
     **/

    return( modulename);

} /** End of 'get_module_basename' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		Update_LoadedList				     **
 ** 									     **
 **   Description:	Add or remove the passed modulename and filename to/ **
 **			from LOADEDMODULES and _LMFILES_		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp      *interp		Attached Tcl Interp. **
 **			char            *modulename	Name of the module   **
 **			char            *filename	Full path name of the**
 **							related modulefile   **
 **									     **
 **   Result:		int	1	Successfull operation		     **
 **									     **
 **   Attached Globals:	flags		Controls whether the modulename      **
 **					should be added (M_XXXX) or removed  **
 **					(M_REMOVE) from the list of loaded   **
 **					modules				     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int Update_LoadedList(	Tcl_Interp	*interp,
			char		*modulename,
			char		*filename)
{
    char 	*argv[4];
    char	*basename;
    char	*module;
 
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_Update_LoadedList, NULL);
#endif

    /**
     **  Apply changes to LOADEDMODULES first
     **/

    argv[1] = "LOADEDMODULES";
    argv[2] = modulename;
    argv[3] = NULL;
    
    if(flags & M_REMOVE) {
	argv[0] = "remove-path";
	cmdRemovePath( 0, interp, 3, argv);
    } else {
	argv[0] = "append-path";
	cmdSetPath( 0, interp, 3, argv);
    }
 
    /**
     **  Apply changes to _LMFILES_ now
     **/

    argv[1] = "_LMFILES_";
    argv[2] = filename;
    argv[3] = NULL;

    if(flags & M_REMOVE) {
	argv[0] = "remove-path";
	cmdRemovePath( 0, interp, 3, argv);
    } else {
	argv[0] = "append-path";
	cmdSetPath( 0, interp, 3, argv);
    }

    /**
     **  A module with just the basename might have been added and now we're
     **  removing one of its versions. We'll want to look for the basename in
     **  the path too.
     **/

    if( flags & M_REMOVE) {
	module = strdup( modulename);
	basename = module;
	if( basename = get_module_basename( basename)) {
	argv[2] = basename;
	argv[0] = "remove-path";
	cmdRemovePath( 0, interp, 3, argv);
	}
	free( module);
    }

    /**
     **  Return on success
     **/

    return( 1);

} /** End of 'Update_LoadedList' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		ForceBasePath					     **
 ** 									     **
 **   Description:	Remove and than add the passed value from/to the     **
 **			passed variable. After removal, the module-path is   **
 **			APPENDED to the passed variable if 'ForceBasePath'   **
 **			has been called and PREPENDED if it was 	     **
 **			'ForceSacredPath'				     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	Tcl_Interp      *interp		Attached Tcl interpr.**
 **			char            *variable_name	Attached variable    **
 **			char            *force_pathname Name of the path to  **
 **							be removed/added     **
 **									     **
 **   Result:		int	1	Successfull operation		     **
 **									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int ForceSacredPath(	Tcl_Interp	*interp, 
                  	char		*variable_name, 
                  	char		*force_pathname)
{
    return( ForcePath( interp, variable_name, force_pathname, 0));
}

int ForceBasePath(	Tcl_Interp	*interp, 
                  	char		*variable_name, 
                  	char		*force_pathname)
{
    return( ForcePath( interp, variable_name, force_pathname, 1));
}

static int ForcePath(	Tcl_Interp	*interp, 
                  	char		*variable_name, 
                  	char		*force_pathname,
			int		 append)
{
    char	*argv[4];
    
#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_ForcePath, NULL);
#endif

    /**
     **  If no pathname to be forced is specified, success is suggested
     **/

    if(	force_pathname == NULL)
	return( 1);

    /**
     **  Set up an according environment and call the command functions
     **/

    argv[1] = variable_name;
    argv[2] = force_pathname;
    argv[3] = NULL;

    /**
     **  First remove the pathname that we're forcing...
     **/

    argv[0] = "remove-path";
    cmdRemovePath( 0, interp, 3, argv);

    /**
     **  Next, add it back to the very end of the list
     **/

    argv[0] = append ? "append-path" : "prepend-path";
    cmdSetPath( 0, interp, 3, argv);

    /**
     **  Return on success 
     **/

    return( 1);

} /** End of 'ForcePath' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		check_magic					     **
 ** 									     **
 **   Description:	Check the magic cookie of the file passed as para-   **
 **			meter if it is a valid module file		     **
 **			Based on check_magic in Richard Elling's	     **
 **			find_by_magic <Richard.Elling"@eng.auburn.edu>	     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	char   *filename	Name of the file to check    **
 **			char   *magic_name	Magic cookie		     **
 **			int     magic_len	Length of the magic cookie   **
 ** 									     **
 **   Result:		int	0	Magic cookie doesn't match or any    **
 **					I/O error			     **
 **				1	Success - Magic cookie has matched   **
 ** 									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

int check_magic( char	*filename,
		 char	*magic_name,
		 int	 magic_len)
{
    int  fd;				/** File descriptor for reading in   **/
    int  read_len;			/** Number of bytes read	     **/
    char buf[BUFSIZ];			/** Read buffer			     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_check_magic, NULL);
#endif

    /**
     **  Parameter check. The length of the magic cookie shouldn't exceed the
     **  length of out read buffer
     **/

    if( magic_len > BUFSIZ)
	return 0;

    /**
     **  Open the file and read in as many bytes as required for checking the
     **  magic cookie. If there's an I/O error (Unable to open the file or
     **  less than magic_len have been read) return on failure.
     **/

    if( -1 == (fd = open( filename, O_RDONLY)))
	if( OK != ErrorLogger( ERR_OPEN, LOC, filename, "reading", NULL))
	    return( 0);			/** -------- EXIT (FAILURE) -------> **/

    read_len = read( fd, buf, magic_len);
    
    if( -1 == close(fd))
	if( OK != ErrorLogger( ERR_CLOSE, LOC, filename, NULL))
	    return( 0);			/** -------- EXIT (FAILURE) -------> **/

    if( read_len < magic_len)
	return( 0);

    /**
     **  Check the magic cookie now
     **/

    return( !strncmp( buf, magic_name, magic_len));

} /** end of 'check_magic' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		cleanse_path					     **
 ** 									     **
 **   Description:	Copy the passed path into the new path buffer and    **
 **			devalue '.' and '+'				     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	const char   *path	Original path		     **
 **			      char   *newpath	Buffer for to copy the new   **
 **						path in			     **
 **			      int     len	Max length of the new path   **
 **									     **
 **   Result:		newpath		will be filled up with the new, de-  **
 **					valuated path			     **
 **									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

void cleanse_path( const char	*path,
		         char	*newpath,
			 int	 len)
{
    unsigned int path_len = strlen( path);	/** Length of the orig. path **/
    int 	 i,				/** Read index		     **/
    		 j;				/** Write index		     **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_cleanse_path, NULL);
#endif

    /**
     **  Stopping at (len - 1) ensures that the newpath string can be
     **  null-terminated below.
     **/

    for( i=0, j=0; i<path_len && j<(len - 1); i++, j++) {

        switch(*path) {
            case '.':
            case '+':
		*newpath++ = '\\';		/** devalue '.' and '+'	    **/
		j++;
		break;
        }

	/**
	 **  Flush the current charater into the newpath buffer
	 **/

        *newpath++ = *path++;

    } /** for **/

    /**
     **  Put a string terminator at the newpaths end
     **/

    *newpath = '\0';

} /** End of 'cleanse_path' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		chop						     **
 ** 									     **
 **   Description:	Remove '\n' characters from the passed string	     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	char   *string	String to be chopped		     **
 **									     **
 **   Result:		string		The chopped string		     **
 **									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

static	char *chop( const char	*string)
{
    char	*s, *t;			/** source and target pointers       **/

#if WITH_DEBUGGING_UTIL_2
    ErrorLogger( NO_ERR_START, LOC, _proc_chop, NULL);
#endif

    /**
     **  Remove '\n'
     **/

    s = t = (char *) string;
    while( *s) {
	if( '\n' == *s)
	    s++;
	else
	    *t++ = *s++;
    }

    /**
     **  Copy the trailing terminator and return
     **/

    *t++ = '\0';
    return( (char *) string);

} /** End of 'chop' **/

#ifndef HAVE_STRDUP

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		strdup						     **
 ** 									     **
 **   Description:	Makes new space to put a copy of the given string    **
 **			into and then copies the string into the new space.  **
 **			Just like the "standard" strdup(3).		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:							     **
 **   Result:								     **
 **   Attached Globals:							     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

char	*strdup( char *str)
{
    int len = strlen( str) + 1;
    char* new = (char *) malloc( len);
    strcpy( new, str);
    return( new);
}
#endif /* HAVE_STRDUP  */

#ifndef HAVE_STRTOK

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		strtok						     **
 ** 									     **
 **   Description: 	Considers the string s1 to consist of a sequence of  **
 **			zero or more text tokens separated by spans of one   **
 **			or more characters from the separator string  s2.    **
 **			Just like the "standard" strtok(3).		     **
 **									     **
 **   Note:		This function is from the Berkeley BSD distribution. **
 **			It was modified to fit our needs!		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:							     **
 **   Result:								     **
 **   Attached Globals:							     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char *strtok(	char		*s,
		const char	*delim)
{
	register char *spanp;
	register int c, sc;
	char *tok;
	static char *last;


	if( s == NULL && (s = last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for( spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if( !c) {		/* no non-delimiter characters */
		last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */

} /** End of 'strtok' **/
#endif

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		chk4spch					     **
 ** 									     **
 **   Description:	goes through the given string and changes any non-   **
 **			printable characters to question marks.		     **
 ** 									     **
 **   First Edition:	91/10/23					     **
 ** 									     **
 **   Parameters:	char	*s		String to be checke	     **
 ** 									     **
 **   Result:		*s			Will be changed accordingly  **
 ** 									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/

void chk4spch(char* s)
{
    for( ; *s; s++)
	if( !isgraph( *s)) *s = '?';

} /** End of 'chk4spch' **/

/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		xdup						     **
 ** 									     **
 **   Description:	will return a string with 1 level of environment     **
 ** 			variables expanded. The limit is MOD_BUFSIZE.	     **
 ** 			An env.var. is denoted with either $name or ${name}  **
 **			\$ escapes the expansion and substitutes a '$' in    **
 **			its place.					     **
 ** 									     **
 **   First Edition:	2000/01/21					     **
 ** 									     **
 **   Parameters:	char	*string		Environment variable	     **
 ** 									     **
 **   Result:		char    *		An allocated string	     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/


char *xdup(char const *string) {
	char *result = NULL;
	char *dollarptr;

	if (string == (char *)NULL) return result;

	/** need to work from copy of string **/
	result = strdup(string);
	/** check for '$' else just pass strdup of it **/
	if ((dollarptr = strchr(result, '$')) == (char *)NULL) {
		return result;
	} else {
	/** found something **/
		char const *envvar;
		char buffer[MOD_BUFSIZE];
		char oldbuffer[MOD_BUFSIZE];
		size_t blen = 0;	/** running buffer length	**/
		char *slashptr = result;/** where to continue parsing	**/
		char  slashchr;		/** store slash char		**/ int brace;		/** flag if ${name}		**/

		/** zero out buffers */
		memset(   buffer, '\0', MOD_BUFSIZE);
		memset(oldbuffer, '\0', MOD_BUFSIZE);

		/** copy everything upto $ into old buffer **/
		*dollarptr = '\0';
		strncpy(oldbuffer, slashptr, MOD_BUFSIZE);
		*dollarptr = '$';

		while (dollarptr) {
			if (*oldbuffer) strncpy(buffer, oldbuffer, MOD_BUFSIZE);
			blen = strlen(buffer);

			/** get the env.var. name **/
			if (*(dollarptr + 1) == '{') {
				brace = 1;
				slashptr = strchr(dollarptr + 1, '}');
			} else {
				slashptr = dollarptr + 1
					+ strcspn(dollarptr + 1,"/:$\\");
				brace = 0;
			}
			if (*slashptr) {
				slashchr = *slashptr;
				*slashptr = '\0';
			} else slashptr = (char *)NULL;

			/** see if escaped **/
			if ((result != dollarptr) && *(dollarptr - 1) == '\\') {
				/** replace \ with 0 and copy rest of name **/
				buffer[blen - 1] = '\0';
				strncat(buffer, dollarptr, MOD_BUFSIZE-blen);
				blen = strlen(buffer);
				if(brace)
					strncat(buffer,"}",MOD_BUFSIZE-blen-1);
			} else {
				/** get env.var. value **/
				envvar = getenv(dollarptr + 1 +brace);

				/** cat env.var. value to rest of string **/
				if (envvar)
					strncat(buffer,envvar,
					MOD_BUFSIZE-blen-1);
			}
			blen = strlen(buffer);

			/** start at slashptr and find next $ **/
			if (slashptr) {
				*slashptr = slashchr;
				dollarptr = strchr(slashptr, '$');
				/** copy everything upto $ **/
				if (dollarptr) *dollarptr = '\0';
				strncat(buffer, slashptr + brace,
					MOD_BUFSIZE -blen -1);
				if (dollarptr) {
					*dollarptr = '$';
					strncpy(oldbuffer, buffer, MOD_BUFSIZE);
				}
			} else {		/** no more to show **/
				dollarptr = (char *)NULL;
			}
		}
		free(result);
		return strdup(buffer);
	}

} /** End of 'xdup' **/
/*++++
 ** ** Function-Header ***************************************************** **
 ** 									     **
 **   Function:		xgetenv						     **
 ** 									     **
 **   Description:	will return an expanded environment variable.	     **
 ** 			However, it will only expand 1 level.		     **
 ** 			See xdup() for details.				     **
 ** 									     **
 **   First Edition:	2000/01/18					     **
 ** 									     **
 **   Parameters:	char	*var		Environment variable	     **
 ** 									     **
 **   Result:		char    *		An allocated string	     **
 ** 									     **
 **   Attached Globals:	-						     **
 ** 									     **
 ** ************************************************************************ **
 ++++*/


char *xgetenv(char const * var) {
	char *result = NULL;

	if (var == (char *)NULL) return result;

	return xdup(getenv(var));

} /** End of 'xgetenv' **/