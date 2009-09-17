
#include <stdio.h>
#include <ctype.h>

unsigned char buff[32000];

char i_name[30] = "i_";
char o_name[30];

void dump( char *fname )
{
	FILE *fp = fopen( fname, "rb" );
	int i, n, a, st, ed;
	if ( fp == NULL ) {
		return;
	}
	n = fread( buff, 1, sizeof(buff), fp );
	fclose( fp );

	for ( i = 0; i < sizeof(i_name)-3; i++ ) {
		a = fname[i];
		if ( !isalnum(a) && a != '_' ) break;
		i_name[i+2] = a;
	}

	for ( i = 44; i < n; i++ ) {
		a = buff[i] - 0x80;
		if ( a ) break;
	}

	if ( i > 44 ) st = i-1; else st = 44;

	for ( i = n-1; i > 44; i++ ) {
		a = buff[i] - 0x80;
		if ( a ) break;
	}

	if ( i < n-1 ) ed = i+1; else ed = n-1;

	if ( ed > st ) {
		sprintf( o_name, "%s.c", i_name );

		fp = fopen( o_name, "wt" );
		if ( fp == NULL ) {
			return;
		}

		n = ed - st;
		fprintf( fp, "#include <musdef.h>\n\n" );
		fprintf( fp, "static const signed char wd%s[] = {\n", i_name );
		for ( i = 0; i < n; i++ ) {
			fprintf( fp, "%d,", buff[st+i]-0x80 );
			if ( (i&7)==7 ) fprintf( fp, "\n" );
		}
		if ( (i&7)!=0 ) fprintf( fp, "\n" );
		fprintf( fp, "};\n\n" );
		fprintf( fp, "const INST %s = {\n", i_name );
		fprintf( fp, "\t0,\n" );
		fprintf( fp, "\t0,\n" );
		fprintf( fp, "\t0,\n" );
		fprintf( fp, "\t0,\n" );
		fprintf( fp, "\tPITCH16K,\n" );
		fprintf( fp, "\tPITCH_C4,\n" );
		fprintf( fp, "\t(signed char *)wd%s,\n", i_name );
		fprintf( fp, "\t0,\n" );
		fprintf( fp, "\t%d<<14,\n", n );
		fprintf( fp, "};\n" );

		fclose( fp );
	}
}




void main( int argc, char *argv[] )
{
	if ( argc >= 2 ) dump( argv[1] );
}

