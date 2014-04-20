#ifndef TCLDEF_H
#define TCLDEF_H

#define TCL_RETURN_STRING(__string)                         \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp))                             \
        {                                                   \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            if (__string)                                   \
            {                                               \
                Tcl_SetStringObj(objp,__string,-1);         \
                Tcl_SetObjResult(interp, objp);             \
                Tcl_DecrRefCount(objp);                     \
                return TCL_OK;                              \
            }                                               \
            Tcl_SetStringObj(objp,"",-1);                   \
            Tcl_SetObjResult(interp, objp);                 \
            Tcl_DecrRefCount(objp);                         \
        }                                                   \
        else                                                \
        {                                                   \
            if (__string)                                   \
            {                                               \
                Tcl_SetStringObj(objp,__string,-1);         \
                return TCL_OK;                              \
            }                                               \
            Tcl_SetStringObj(objp,"",-1);                   \
        }                                                   \
        return TCL_OK;                                      \
    }
#define TCL_RETURN_ERROR(__error)                           \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp))                             \
        {                                                   \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            if (__error)                                    \
            {                                               \
                Tcl_SetStringObj(objp,__error,-1);          \
                Tcl_SetObjResult(interp, objp);             \
                Tcl_DecrRefCount(objp);                     \
                return TCL_ERROR;                           \
            }                                               \
            Tcl_SetStringObj(objp,"",-1);                   \
            Tcl_SetObjResult(interp, objp);                 \
            Tcl_DecrRefCount(objp);                         \
        }                                                   \
        else                                                \
        {                                                   \
            if (__error)                                    \
            {                                               \
                Tcl_SetStringObj(objp,__error,-1);          \
                return TCL_ERROR;                           \
            }                                               \
            Tcl_SetStringObj(objp,"",-1);                   \
        }                                                   \
        return TCL_ERROR;                                   \
    }
#define TCL_RETURN_INT(__int)                               \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp)) {                           \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            Tcl_SetIntObj(objp,__int);                      \
            Tcl_SetObjResult(interp,objp);                  \
            Tcl_DecrRefCount(objp);                         \
        } else                                              \
            Tcl_SetIntObj(objp,__int);                      \
        return TCL_OK;                                      \
    }
#define TCL_RETURN_WIDEINT(__int)                           \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp)) {                           \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            Tcl_SetWideIntObj(objp,__int);                  \
            Tcl_SetObjResult(interp,objp);                  \
            Tcl_DecrRefCount(objp);                         \
        } else                                              \
            Tcl_SetWideIntObj(objp,__int);                  \
        return TCL_OK;                                      \
    }
#define TCL_RETURN_ERROR_INT(__int)                         \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp)) {                           \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            Tcl_SetIntObj(objp,__int);                      \
            Tcl_SetObjResult(interp,objp);                  \
            Tcl_DecrRefCount(objp);                         \
        } else                                              \
            Tcl_SetIntObj(objp,__int);                      \
        return TCL_ERROR;                                   \
    }
#define TCL_RETURN_BYTEARRAY(__data, __len)                 \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp)) {                           \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            Tcl_SetByteArrayObj(objp,__data, __len);        \
            Tcl_SetObjResult(interp,objp);                  \
            Tcl_DecrRefCount(objp);                         \
        } else                                              \
            Tcl_SetByteArrayObj(objp,__data, __len);        \
        return TCL_OK;                                      \
    }
#define TCL_RETURN_BYTEARRAY_FREE(__data, __len)            \
    {                                                       \
        objp = Tcl_GetObjResult(interp);                    \
        if (Tcl_IsShared(objp)) {                           \
            objp = Tcl_DuplicateObj(objp);                  \
            Tcl_IncrRefCount(objp);                         \
            Tcl_SetByteArrayObj(objp,__data, __len);        \
            Tcl_SetObjResult(interp,objp);                  \
            Tcl_DecrRefCount(objp);                         \
        } else                                              \
            Tcl_SetByteArrayObj(objp,__data, __len);        \
        Tcl_Free((char*)__data);                            \
        return TCL_OK;                                      \
    }

#endif

