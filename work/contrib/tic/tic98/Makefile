BIN=/home/singlis/linux/bin
IMAGES=/home/ml/singlis
LIBTIC= 
LIBS=   -lm #-L/home/singlis/linux/lib -lccmalloc -ldl
CC=	gcc -pipe 
CPP=	gcc -pipe

#CFLAGS=-O -g -Wall
CFLAGS= -O -Wall -Wno-unused

EXES=tic98 tic98l b_gamma_enc b_gamma_dec q_gamma_enc q_gamma_dec \
	rotate_image random_image pbmtohough ppmd_enc ppmd_dec\
	pbm_to_node pbm_to_coords pbmclean page_features rls zone_highlight \
	char_features pbmtomarks labelmarks

all: tic98 ppmd_enc ppmd_dec b_gamma_enc b_gamma_dec

all2: $(EXES)

.SUFFIXES: .cc .c

.cc.o:
	$(CPP) $(CFLAGS) -c $*.cc 
.c.o:
	$(CC) $(CFLAGS) -c $*.c 

exp:
	(cd results; make -f Makefile)

install: all
	install $(EXES) $(BIN)

clean: 
	rm -f *~ core *.o *.bak images/*.cmp $(EXES) #main_tic_test

ppmd_enc:	ppmd_enc.o ppmd_model.o ppmd_hash.o arithcode.o getopt.o
	$(CC) -o $@ $^ $(LIBS)

ppmd_dec:	ppmd_dec.o ppmd_model.o ppmd_hash.o arithcode.o getopt.o
	$(CC) -o $@ $^ $(LIBS)

b_gamma_enc:	b_gamma_enc.o arithcode.o getopt.o b_gamma.o globals.o
	$(CC) -o $@ $^ $(LIBS)

convert:	convert.o arithcode.o getopt.o b_gamma.o globals.o
	$(CC) -o $@ $^

deconvert:	deconvert.o arithcode.o getopt.o b_gamma.o globals.o
	$(CC) -o $@ $^

b_gamma_dec:	b_gamma_dec.o arithcode.o getopt.o b_gamma.o globals.o 
	$(CC) -o $@ $^ $(LIBS)

q_gamma_enc: b_gamma.o b_gamma_enc.o arithcode.o globals.o
	$(CC) $(CFLAGS) -o $@ $^

q_gamma_dec: b_gamma.o b_gamma_dec.o arithcode.o globals.o
	$(CC) $(CFLAGS) -o $@ $^

rls:	rls.o marklist.o pbmtools.o math-utils.o utils.o boundary.o
	$(CC) -o $@ $^ $(LIBS)

zone_highlight:	zone_highlight.o marklist.o pbmtools.o math-utils.o utils.o boundary.o
	$(CC) -o $@ $^ $(LIBS)

tic98:	main_tic.o tic98.o codebook.o context.o b_gamma.o sort_lines.o \
		boundary.o marklist.o pbmtools.o template_match.o \
		arithcode.o utils.o getopt.o vector.o windowing.o globals.o \
		timer.o rotate.o docstrum.o line.o ppmd_model.o ppmd_hash.o \
		codebilevel.o 
	$(CC) -o $@ $^ $(LIBS) 

page_features:	page_features.o marklist.o pbmtools.o boundary.o\
		utils.o getopt.o globals.o rotate.o 
	$(CC) -o $@ $^ $(LIBS) 

char_features:	char_features.o marklist.o pbmtools.o boundary.o\
		utils.o getopt.o globals.o rotate.o  template_match.o context.o arithcode.o
	$(CC) -o $@ $^ $(LIBS) 

pbmclean:	pbmclean.o boundary.o marklist.o pbmtools.o utils.o
	$(CC) -o $@ $^ $(LIBS) 

pbmtomarks:	pbmtomarks.o boundary.o marklist.o pbmtools.o utils.o
	$(CC) -o $@ $^ $(LIBS) 

labelmarks:	labelmarks.o boundary.o marklist.o pbmtools.o utils.o
	$(CC) -o $@ $^ $(LIBS) 

tic98l:	main_tic.o tic98l.o codebook.o context.o b_gamma.o sort_lines.o \
		boundary.o marklist.o pbmtools.o template_match.o \
		arithcode.o utils.o getopt.o vector.o windowing.o globals.o \
		timer.o rotate.o docstrum.o line.o ppmd_model.o ppmd_hash.o \
		codebilevel.o 
	$(CC) -o $@ $^ $(LIBS) 

pbmtohough: pbmtohough.o hough.o marklist.o math-utils.o windowing.o double_array.o pbmtools.o utils.o boundary.o
	$(CC) -o $@ $^ $(LIBS)

random_image: random_image.o hough.o marklist.o math-utils.o windowing.o double_array.o pbmtools.o utils.o boundary.o
	$(CC) -o $@ $^ $(LIBS)

regress: main_tic
	./main_tic -c -i ${IMAGES}/uw_fifty_300/A006.pbm -o cmp -8N -R Howard; cmp cmp regression/A006.regress
	./main_tic -c -i $(IMAGES)/uw_fifty_300/S048.pbm -o cmp -8N -R Howard; cmp cmp regression/S048.regress
	\rm cmp

check: main_tic
	./main_tic -c -i ${IMAGES}/uw_fifty_300/A006.pbm -o cmp -f images/A006
	./main_tic -d -i cmp -o cmp
	cmp cmp.001 ${IMAGES}/uw_fifty_300/A006.pbm

test: test_1 test_2 test_3 test_4 test_5

test_1: main_tic random_image	
	cp ./main_tic ./main_tic_test
	test=0;failed=0;while [ 1 ]; do (./random_image >/tmp/random.1.pbm; \
	date;\
	./main_tic_test -c -i /tmp/random.1.pbm -o /tmp/cmp.1 -m B_AVG;\
	./main_tic_test -d -i /tmp/cmp.1 -o /tmp/random.1.decmp -m B_AVG;\
	cmp /tmp/random.1.pbm /tmp/random.1.decmp.001;); if [ $$? -eq 1 ]; then failed=`expr $$failed + 1`;fi; test=`expr $$test + 1`; echo passed: $$test, failed: $$failed; done 

test_2: main_tic random_image	
	sleep 2;
	test=0;failed=0;while [ 1 ]; do (./random_image >/tmp/random.2.pbm; \
	date;\
	./main_tic_test -c -i /tmp/random.2.pbm -o /tmp/cmp.2 -m F_AVG ;\
	./main_tic_test -d -i /tmp/cmp.2 -o /tmp/random.2.decmp -m F_AVG;\
	cmp /tmp/random.2.pbm /tmp/random.2.decmp.001;); if [ $$? -eq 1 ]; then failed=`expr $$failed + 1`;fi; test=`expr $$test + 1`; echo passed: $$test, failed: $$failed; done 

test_3: main_tic random_image	
	sleep 2;
	test=0;failed=0;while [ 1 ]; do (./random_image >/tmp/random.3.pbm; \
	date;\
	./main_tic_test -c -i /tmp/random.3.pbm -o /tmp/cmp.3 -m B_SINGLE;\
	./main_tic_test -d -i /tmp/cmp.3 -o /tmp/random.3.decmp -m B_SINGLE;\
	cmp /tmp/random.3.pbm /tmp/random.3.decmp.001;); if [ $$? -eq 1 ]; then failed=`expr $$failed + 1`;fi; test=`expr $$test + 1`; echo passed: $$test, failed: $$failed; done 

test_4: main_tic random_image	
	sleep 2;
	test=0;failed=0;while [ 1 ]; do (./random_image >/tmp/random.4.pbm; \
	date;\
	./main_tic_test -c -i /tmp/random.4.pbm -o /tmp/cmp.4 -m F_SINGLE;\
	./main_tic_test -d -i /tmp/cmp.4 -o /tmp/random.4.decmp -m F_SINGLE;\
	cmp /tmp/random.4.pbm /tmp/random.4.decmp.001;); if [ $$? -eq 1 ]; then failed=`expr $$failed + 1`;fi; test=`expr $$test + 1`; echo passed: $$test, failed: $$failed; done 

test_5: main_tic random_image	
	sleep 2;
	test=0;failed=0;while [ 1 ]; do (./random_image >/tmp/random.5.pbm; \
	date;\
	./main_tic_test -c -i /tmp/random.5.pbm -o /tmp/cmp.5 -m HYBRID;\
	./main_tic_test -d -i /tmp/cmp.5 -o /tmp/random.5.decmp -m HYBRID;\
	cmp /tmp/random.5.pbm /tmp/random.5.decmp.001;); if [ $$? -eq 1 ]; then failed=`expr $$failed + 1`;fi; test=`expr $$test + 1`; echo passed: $$test, failed: $$failed; done 


main_skew: main_skew.o docstrum.o marklist.o pbmtools.o utils.o \
	 boundary.o getopt.o windowing.o
	$(CC) -o $@ $^ $(LIBS)

rotate_image: rotate_image.o rotate.o marklist.o pbmtools.o utils.o
	$(CC) -o $@ $^ $(LIBS)

pbm_to_coords: pbm_to_coords.o marklist.o pbmtools.o utils.o \
	 boundary.o getopt.o windowing.o
	$(CC) -o $@ $^ $(LIBS)

pbm_to_node: pbm_to_node.o marklist.o pbmtools.o utils.o \
	 boundary.o getopt.o windowing.o
	$(CC) -o $@ $^ $(LIBS)

coords_profile: coords_profile.o  marklist.o pbmtools.o utils.o \
	 boundary.o getopt.o windowing.o
	$(CC) -o $@ $^ $(LIBS)

coords_baird: coords_baird.o marklist.o pbmtools.o utils.o \
	 boundary.o getopt.o windowing.o
	$(CC) -o $@ $^ $(LIBS)

coords_docstrum: coords_docstrum.o docstrum.o marklist.o pbmtools.o utils.o \
	 boundary.o getopt.o windowing.o
	$(CC) -o $@ $^ $(LIBS)

 