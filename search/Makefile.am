
bin_PROGRAMS = associate print_doc

associate_SOURCES = associate.c files.c query.c neighbors.c \
                    arrayoutput.c list.c $(top_srcdir)/lib/utils.c toc.c \
                    $(top_srcdir)/lib/model_params.c search_utils.c

print_doc_SOURCES = print_doc.c toc.c $(top_srcdir)/lib/model_params.c \
		    search_utils.c

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/admin

EXTRA_DIST = .deps arrayoutput.h files.h neighbors.h search_types.h \
             associate.h list.h query.h toc.h search_utils.h

