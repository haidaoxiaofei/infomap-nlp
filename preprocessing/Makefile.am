
bin_PROGRAMS = prepare_corpus count_wordvec encode_wordvec \
               count_artvec write_text_params

prepare_corpus_SOURCES = prepare_corpus.c word_tree.c tokenizer.c stopper.c \
		         $(top_srcdir)/lib/utils.c report.c \
			 $(top_srcdir)/lib/model_params.c

count_artvec_SOURCES = count_artvec.c wordlist.c dict.c \
                       word_tree.c $(top_srcdir)/lib/utils.c \
		       vector.c matrix.c \
                       $(top_srcdir)/lib/model_params.c

count_wordvec_SOURCES = count_wordvec.c wordlist.c dict.c word_tree.c \
                        $(top_srcdir)/lib/utils.c vector.c matrix.c \
			$(top_srcdir)/lib/model_params.c

encode_wordvec_SOURCES =  encode_wordvec.c dict.c word_tree.c \
                          $(top_srcdir)/lib/utils.c \
                          report.c vector.c $(top_srcdir)/lib/model_params.c

write_text_params_SOURCES = write_text_params.c \
			    $(top_srcdir)/lib/model_params.c

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/admin

EXTRA_DIST = .deps count_artvec.h count_wordvec.h dict.h \
	     encode_wordvec.h matrix.h prepare_corpus.h \
	     preprocessing_env.h preprocessing_types.h \
	     report.h stopper.h tokenizer.h vector.h \
	     word_tree.h wordlist.h
