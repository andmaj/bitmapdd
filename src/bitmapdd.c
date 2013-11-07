/**
 * Creates a bitmap from a file (or device).
 * 
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 * 
 * Report bugs to <majdan.andras@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <netinet/in.h>
#include <time.h>

#ifndef NOSIGNAL
#include <signal.h>
#endif

#define DEFAULT_BS	512
#define UINT_BITS	sizeof(unsigned int) * CHAR_BIT

#define VERSION "1.1.1"

unsigned long long in_block= 0, in_bytes = 0;
unsigned long long out_block= 0, out_bytes = 0;

int run;

time_t start_time;

void print_progress(FILE *out)
{
	time_t end_time;
	double s;
	
	time(&end_time);
	s =  difftime(end_time, start_time);
	
	fprintf(out, 
			"%llu blocks in (%llu bytes)\n"
			"%llu blocks out (%llu bytes)\n"
			"%.0f s, %.1f MB/s\n",
			in_block, in_bytes, out_block, out_bytes, s, 
			(double)in_bytes/s/(double)1000000);
}

#ifndef NOSIGNAL
void sig_handler(int signum)
{
	if(signum==SIGUSR1) print_progress(stderr);
	if(signum==SIGINT) run = 0;
}
#endif

size_t contains1(char *block, int res, char nullbyte)
{
	while(res--)
	{
		if(block[res] != nullbyte) return 1;
	}
	return 0;
}

int bitmapdd(FILE *in, FILE *out, int count_set, 
			 size_t count, size_t bs, char nullbyte)
{
	#ifndef NOSIGNAL
	signal(SIGUSR1, sig_handler);
	signal(SIGINT, sig_handler);
	#endif
	
	time(&start_time);
	
	unsigned int bits = 0;  /* Holder of bits */
	int cbi = 0; 			/* Current bit index */
	
	char *block = malloc(bs);
	if(block==NULL)
	{
		fprintf(stderr, "Memory allocation has failed.\n");
		exit(EXIT_FAILURE);
	}

	size_t res;
	int err;
	err = 0;
	run = 1;
	
	while( run && (!count_set || count--) )
	{
		res = fread(block, sizeof(char), bs, in);
		if(res!=bs)
		{
			if( feof(in) )
				run = 0;
			else
			{
				fprintf(stderr, "Input I/O error\n");
				err = 1;
				break;
			}
		}
 
		if(res!=0) ++in_block;
		
		in_bytes += res;
		
		res = contains1(block, res, nullbyte);
		bits += res << (UINT_BITS - cbi - 1);
		
		if(++cbi == UINT_BITS || (!run && cbi) )
		{	
			cbi = 0;
			bits = htonl(bits);
			res = fwrite(&bits, sizeof(unsigned int), 1, out);
			if(res!=1)
			{
				fprintf(stderr, "Output I/O error\n");
				err = 1;
				break;
			}
			++out_block;
			out_bytes += sizeof(unsigned int);
			bits = 0;
		}
	}
	
	if(out != stdout) print_progress(stdout);
	
	fclose(in);
	fclose(out);
	
	free(block);
	
	if(err) return 1;
	else return 0;
}

int main(int argc, char *argv[])
{
	int c;
	char *endptr;
	
	int count_set;
	char nullbyte;
	unsigned long int count, bs, tmp;
	FILE *in, *out;
	
	/* Default values */
	/* Count not set, so count till end */
	count_set = 0;
	nullbyte = '\0';
	bs = DEFAULT_BS;
	in = stdin;
	out = stdout;
 
	while(1)
	{
		static struct option long_options[] =
		{
			{"null",	required_argument, 0, 'n'},
			{"count",	required_argument, 0, 'c'},
			{"bs",		required_argument, 0, 'b'},
			{"if", 		required_argument, 0, 'i'},
			{"of",    	required_argument, 0, 'o'},
			{"help",    no_argument, 	   0, 'h'},
			{"version", no_argument, 	   0, 'v'},
			{0, 0, 0, 0}
		};
		
		int option_index = 0;
     
		c = getopt_long(argc, argv, "n:c:b:i:o:hv",
										long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;
     
		switch (c)
		{
			case 'n':
				errno = 0;
				tmp = strtoul(optarg, &endptr, 10);
				if(tmp > CHAR_MAX || *endptr!='\0' || errno!=0)
				{
					fprintf(stderr,
							"Wrong argument for count: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				nullbyte = tmp;
				break;
			case 'c':
				errno = 0;
				count = strtoul(optarg, &endptr, 10);
				if(*endptr!='\0' || errno!=0)
				{
					fprintf(stderr,
							"Wrong argument for count: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				count_set = 1;
				break;
			case 'b':
				errno = 0;
				bs = strtoul(optarg, &endptr, 10);
				if(*endptr!='\0' || errno!=0)
				{
					fprintf(stderr,
							"Wrong argument for bs: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'i':
				in = fopen(optarg, "r+b");
				if(in==NULL)
				{
					if(out!=stdout) fclose(out);
					fprintf(stderr,
							"Cannot open input file: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;

			case 'o':
				out = fopen(optarg, "w+b");
				if(out==NULL)
				{
					if(in!=stdin) fclose(in);
					fprintf(stderr,
							"Cannot open output file: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'h':
				printf(
					"Usage: bitmapdd [options]\n\n"
					
					"Creates a bitmap from a file (or device).\n\n"
					
					"Options:\n"
					"  --null NUM   Character code of null (default: 0)\n"
					"  --bs NUM     Block size (default: 512)\n"
					"  --count NUM  Count of blocks (default: till end of input)\n"
					"  --if FILE    Input file (default: stdin)\n"
					"  --of FILE    Output file (default: stdout)\n"
					"  --help       Usage information\n"
					"  --version    Print version\n\n"
	
					"Examples:\n"
					"bitmapdd --if /dev/sda --of usagemap.dat --bs 4096\n\n" 

					"Written by Andras Majdan\n"
					"License: GNU General Public License Version 3\n"
					"Report bugs to <majdan.andras@gmail.com>\n");
				exit (EXIT_SUCCESS);
				break;
			case 'v':
				printf("Version: %s (Compiled on %s)\n", VERSION, __DATE__);
				exit (EXIT_SUCCESS);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				exit(EXIT_FAILURE);
				break;
		}
	}
     
	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		fprintf (stderr, "Invalid option(s): ");
		while (optind < argc)
			fprintf (stderr, "%s ", argv[optind++]);
		fprintf(stderr, "\n");
			
		exit(EXIT_FAILURE);
	}
	
	if( bitmapdd(in, out, count_set, count, bs, nullbyte) )
		exit( EXIT_FAILURE);
	else 
		exit (EXIT_SUCCESS);	
}
