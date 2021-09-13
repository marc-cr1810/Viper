#include "codeobject.h"

#include "bytesarrayobject.h"
#include "tupleobject.h"
#include "stringobject.h"

//
//
//		Methods
//
//

static void code_dealloc(ViCodeObject *self)
{
	ViObject_XDECREF(self->co_code);
	ViObject_XDECREF(self->co_consts);
	ViObject_XDECREF(self->co_names);
	ViObject_XDECREF(self->co_varnames);
	ViObject_XDECREF(self->co_filename);
	ViObject_XDECREF(self->co_name);

	Vi_TYPE(self)->tp_free((ViObject *)self);
}

ViTypeObject ViCodeType = {
	VAROBJECT_HEAD_INIT(&ViCodeType, 0) // base
	"code",								// tp_name
	"Code object type",					// tp_doc
	sizeof(ViCodeObject),				// tp_size
	0,									// tp_itemsize
	TPFLAGS_DEFAULT,					// tp_flags
	(destructor)code_dealloc,			// tp_dealloc
	0,									// tp_number_methods
	0,									// tp_sequence_methods
	0,									// tp_clear
	0,									// tp_base
	0,									// tp_dict
	0,									// tp_new
	Mem_Free							// tp_free
};

ViCodeObject* ViCodeObject_NewEmpty(const char* filename, const char* func_name, Vi_int32_t lineno)
{
	static ViObject* empty_byte_array = NULL;
	static ViObject* null_tuple = NULL;
	ViCodeObject* co = NULL;
	ViObject* filename_ob = NULL;
	ViObject* funcname_ob = NULL;

	if (empty_byte_array == NULL)
	{
		empty_byte_array = ViByteArrayObject_FromString("", 0);
		if (empty_byte_array == NULL)
			goto failed;
	}
	if (null_tuple == NULL)
	{
		null_tuple = ViTupleObject_New(0);
		if (null_tuple == NULL)
			goto failed;
	}

	filename_ob = ViStringObject_FromStringAndSize(filename, sizeof(filename));
	if (filename_ob == NULL)
		goto failed;
	funcname_ob = ViStringObject_FromStringAndSize(func_name, sizeof(func_name));
	if (funcname_ob == NULL)
		goto failed;

	co = ViObject_NEW(ViCodeObject, &ViCodeType);

	co->co_name = funcname_ob;
	co->co_filename = filename_ob;
	co->co_argcount = 0;
	co->co_localvarcount = 0;
	co->co_code = empty_byte_array;
	co->co_consts = null_tuple;
	co->co_names = null_tuple;
	co->co_varnames = null_tuple;
	co->co_lineno = lineno;

	return co;
failed:
	ViObject_XDECREF(filename_ob);
	ViObject_XDECREF(funcname_ob);
	return co;
}