#!/usr/bin/perl

no integer;

if ( $#ARGV != 1 ) {
	print STDERR "Usage: $0 generator_poly key < input_file > output_file\n";
	print STDERR "all numbers in hex\n";
	exit 1;
}

$lfsr_poly = hex($ARGV[0]);
$lfsr_key = hex($ARGV[1]);

undef $/;
binmode STDIN;
binmode STDOUT;

@input_data_array = unpack("C*", <STDIN>);

for ($iByte = 0; $iByte < scalar @input_data_array; $iByte++)
{
	$Byte = $input_data_array[$iByte];
	
	for ($iBit = 0; $iBit < 8; $iBit++)
	{
		$Temp = $lfsr_key & $lfsr_poly;
		$Temp ^= $Temp >> 1 & 0xffffffff;
		$Temp ^= $Temp >> 2 & 0xffffffff;
		$Temp ^= $Temp >> 4 & 0xffffffff;
		$Temp ^= $Temp >> 8 & 0xffffffff;
		$Temp ^= $Temp >> 16 & 0xffffffff;
		$Temp = $Temp & 0x01;
		$lfsr_key <<= 1;
		$lfsr_key &= 0xffffffff;
		$lfsr_key |= $Temp;
	}
	$output_data_array[$iByte] = (($lfsr_key & 0xff) ^ $Byte) & 0xff;
}

$output_packed = pack("C*", @output_data_array);

print STDOUT $output_packed;

