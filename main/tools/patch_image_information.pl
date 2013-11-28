#!/usr/bin/perl



sub _reflectperl {
  my ($in, $width) = @_;
  my $out = 0;
  for(my $i=1; $i < ($width+1); $i++) {
    $out |= 1 << ($width-$i) if ($in & 1);
    $in=$in>>1;
  }
  $out;
}

# Only load the non-XS stuff on demand
defined &_crc or eval <<'ENOXS';

sub _reflect {
  my ($in, $width) = @_;
  my $out = 0;
  for(my $i=1; $i < ($width+1); $i++) {
    $out |= 1 << ($width-$i) if ($in & 1);
    $in=$in>>1;
  }
  $out;
}

sub _tabinit {
  my ($width,$poly_in,$ref) = @_;
  my @crctab;
  my $poly = $poly_in;

  if ($ref) {
    $poly = _reflect($poly,$width);
  }

  for (my $i=0; $i<256; $i++) {
    my $r = $i<<($width-8);
    $r = $i if $ref;
    for (my $j=0; $j<8; $j++) {
      if ($ref) {
	$r = ($r>>1)^($r&1&&$poly)
      } else {
	if ($r&(1<<($width-1))) {
	  $r = ($r<<1)^$poly
	} else {
	  $r = ($r<<1)
	}
      }
    }
    my $x=$r&2**$width-1;
    push @crctab, $x;
  }
  \@crctab;
}

sub _crc {
  my ($message,$width,$init,$xorout,$refin,$refout,$tab) = @_;
  my $crc = $init;
  $crc = _reflect($crc,$width) if $refin;
  my $pos = -length $message;
  my $mask = 2**$width-1;
  while ($pos) {
    if ($refin) {
      $crc = ($crc>>8)^$tab->[($crc^ord(substr($message, $pos++, 1)))&0xff]
    } else {
      $crc = (($crc<<8))^$tab->[(($crc>>($width-8))^ord(substr $message,$pos++,1))&0xff]
    }
  }

  if ($refout^$refin) {
    $crc = _reflect($crc,$width);
  }

  $crc = $crc ^ $xorout;
  $crc & $mask;
}

ENOXS

%_typedef = (
# name,  [width,init,xorout,refout,poly,refin);
  crc8 => [8,0,0,0,0x07,0],
  crcccitt => [16,0xffff,0,0,0x1021,0],
  crc16 => [16,0,0,1,0x8005,1],
  crc32 => [32,0xffffffff,0xffffffff,1,0x04C11DB7,1],
);

sub new {
  my $that=shift;
  my %params=@_;
  my $class = ref($that) || $that;
  my $self = {map { ($_ => $params{$_}) }
                      qw(type width init xorout poly refin refout cont)};
  bless $self, $class;
  $self->reset();
  map { if (defined($params{$_})) { $self->{$_} = $params{$_} } }
                      qw(type width init xorout poly refin refout cont);
  $self
}

sub reset {
  my $self = shift;
  my $typeparams;
  # default is crc32 if no type and no width is defined
  if (!defined($self->{type}) && !defined($self->{width})) {
    $self->{type} = "crc32";
  }
  if (defined($self->{type}) && exists($_typedef{$self->{type}})) {
    $typeparams = $_typedef{$self->{type}};
    $self->{width} = $typeparams->[0],
    $self->{init} = $typeparams->[1],
    $self->{xorout} = $typeparams->[2],
    $self->{refout} = $typeparams->[3],
    $self->{poly} = $typeparams->[4],
    $self->{refin} = $typeparams->[5],
  }
  $self->{_tab} = _tabinit($self->{width}, $self->{poly}, $self->{refin});
  $self->{_data} = undef;
  $self
}


#########################################
# Procedural interface:

sub crc {
  my ($message,$width,$init,$xorout,$refout,$poly,$refin) = @_;
  _crc($message,$width,$init,$xorout,$refin,$refout,_tabinit($width,$poly,$refin));
}

# CRC8
# poly: 07, width: 8, init: 00, revin: no, revout: no, xorout: no

sub crc8 { crc($_[0],@{$_typedef{crc8}}) }

# CRC-CCITT standard
# poly: 1021, width: 16, init: ffff, refin: no, refout: no, xorout: no

sub crcccitt { crc($_[0],@{$_typedef{crcccitt}}) }

# CRC16
# poly: 8005, width: 16, init: 0000, revin: yes, revout: yes, xorout: no

sub crc16 { crc($_[0],@{$_typedef{crc16}}) }

# CRC32
# poly: 04C11DB7, width: 32, init: FFFFFFFF, revin: yes, revout: yes,
# xorout: FFFFFFFF
# equivalent to: cksum -o3

sub crc32 { crc($_[0],@{$_typedef{crc32}}) }




if ($#ARGV != 3)
{
	print STDERR "Usage: $0 binary_file elf_file image_info_base image_info_struct_offset\n";
	print STDERR "all numbers in hex\n";
	exit 1;
}

$binary_file = $ARGV[0];
$elf_file = $ARGV[1];
$image_info_base = hex($ARGV[2]);
$image_info_struct_offset = hex($ARGV[3]);

$binary_file_main_offset = $image_info_base - 0x08000000;


###################### Calculate crc from crc_generation_file
if (open(INPUT, "< $binary_file") == false)
{
	printf "0x%08x\n", 0;
	exit(-1);
}

undef $/;
binmode INPUT;
seek INPUT, $binary_file_main_offset, SEEK_SET;

$binary_file_position = tell INPUT;

if ($binary_file_position != $binary_file_main_offset)
{
	print "File offset error\n";
	exit (-1);
}

$binary_file_data = <INPUT>;
$binary_file_length = length($binary_file_data);


#typedef struct
#{
#    uint32_t    checksum;           // 4
#    uint32_t    size;               // 4
#    uint16_t    hw_id;              // 2
#    uint16_t    hw_revision;        // 2
#    uint8_t     sw_version_major;   // 1
#    uint8_t     sw_version_minor;   // 1
#    uint8_t     reserved_1;         // 1
#    uint8_t     reserved_2;         // 1
#} IMAGE_INFORMATION_T;              // 16

# strip placeholder value
@crc_generation_data_unpacked = unpack("C*", $binary_file_data);
$crc_generation_data_unpacked[$image_info_struct_offset] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 1] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 2] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 3] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 4] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 5] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 6] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 7] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 8] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 9] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 10] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 11] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 12] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 13] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 14] = 0;
$crc_generation_data_unpacked[$image_info_struct_offset + 15] = 0;
$binary_file_data = pack("C*", @crc_generation_data_unpacked);

$checksum = crc32($binary_file_data);

#printf "crc32: 0x%08x\n", $checksum;
#printf "size: %i\n", $binary_file_length;

close INPUT;


###################### Patch binformation into elf file
if (open(OUTPUT, "< $elf_file") == false)
{
	printf "0x%08x\n", 0;
	exit(-1);
}

undef $/;
binmode OUTPUT;
$elf_file_data = <OUTPUT>;
close OUTPUT;

@sequence = ( 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xeb, 0xda, 0xed );
$pos_in_sequence = 0;
$pos_in_file = 0;

@elf_file_data_unpacked = unpack("C*", $elf_file_data);

foreach (@elf_file_data_unpacked)
{
	$pos_in_file += 1;
	if ($_ == $sequence[$pos_in_sequence])
	{
		$pos_in_sequence += 1;
		if ($pos_in_sequence == @sequence)
		{
			push (@found_locations, $pos_in_file - @sequence);
		}
	}
	else
	{
		$pos_in_sequence = 0;
	}
} 

if (@found_locations != 1)
{
	printf "Error location found %i times\n", @found_locations;
	exit(-1);
}

#printf "Location found at 0x%08X\n", $found_locations[0];

$elf_file_data_unpacked[$found_locations[0] + 0] = ($checksum >> 0) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 1] = ($checksum >> 8) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 2] = ($checksum >> 16) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 3] = ($checksum >> 24) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 4] = ($binary_file_length >> 0) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 5] = ($binary_file_length >> 8) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 6] = ($binary_file_length >> 16) & 0xff;
$elf_file_data_unpacked[$found_locations[0] + 7] = ($binary_file_length >> 24) & 0xff;
$elf_file_data = pack("C*", @elf_file_data_unpacked);

if (open(OUTPUT, "> $elf_file") == false)
{
	printf "0x%08x\n", 0;
	exit(-1);
}

undef $/;
binmode OUTPUT;
print OUTPUT $elf_file_data;
close OUTPUT;






