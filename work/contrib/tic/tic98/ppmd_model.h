/* Model module definitions used by both the encoder and the decoder. */

void
init_arguments (int argc, char *argv[]);
void
init_symbols();
void
printFree (FILE *fp);
void
ppm_start_encoding (unsigned int max_order, unsigned int symbol_size);
void
ppm_finish_encoding ();
void
ppm_encode_symbol (unsigned int symbol);
unsigned int
ppm_start_decoding (unsigned int max_order, unsigned int symbol_size);
void
ppm_finish_decoding ();
unsigned int
ppm_decode_symbol();
void
init_ppmd_globals();
unsigned int eof_symbol();
