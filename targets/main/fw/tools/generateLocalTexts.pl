#!/usr/bin/perl

use strict;
my $FileName = shift;
my $LastSlashPos1 = rindex($FileName, "/");
my $LastSlashPos2 = rindex($FileName, "\\");
my $LastSlashPos = ($LastSlashPos1 > $LastSlashPos2) ? $LastSlashPos1 : $LastSlashPos2;
my $Folder = substr($FileName, 0, $LastSlashPos + 1);
my $Name = substr( $FileName, 0, rindex($FileName, ".") );
$Name = substr($Name, $LastSlashPos + 1);

my %Languages;
my @Languages;

if (-e $FileName) {    # read in existing text-file
  open(IN, "<$FileName") or die("ERROR: Cannot open file '$FileName' ($!)");
    my $CurrentLanguage = 'en';
    my $CurrentEnumName = 'local_e';
    $Languages{$CurrentLanguage} = {};
    push @Languages, $CurrentLanguage;
    while (<IN>) {
      my $Line = strip($_);
      if (substr($Line, 0, 1) ne '#') {
        if (substr($Line, 0, 5) eq 'lang=') {
          $CurrentLanguage = substr($Line, 5, 2);
          
          if (!$Languages{$CurrentLanguage}) { 
            $Languages{$CurrentLanguage} = {};
            push @Languages, $CurrentLanguage;
          }
        }
     elsif (substr($Line, 0, 5) eq 'enum=') {
          $CurrentEnumName = substr($Line, 5);
        }
        else {
          my @String = split(" ", $Line);
          my $TextName = shift(@String);
          my $String = join(" ", @String);
          $String =~ s/"/\\"/g;
          unless ( defined($Languages{$CurrentLanguage}->{Entries}->{$TextName}) ) {
            unless ( ref($Languages{$CurrentLanguage}->{TextNames}) ) {
              $Languages{$CurrentLanguage}->{TextNames} = [];
            }
            push @{ $Languages{$CurrentLanguage}->{TextNames} }, $TextName;
          }

          my %Entry = ( Enum => $CurrentEnumName, 
                      String => $String
                      );
          $Languages{$CurrentLanguage}->{Entries}->{$TextName} = \%Entry;
        }
      }
    }
  close(IN);
}
else {                 # create new empty text-file
  print "creating file '$Folder$FileName'\n";
  open(OUT, ">$Folder$FileName") or die("ERROR: Cannot create file '$FileName' ($!)");
  print OUT "# $FileName\n";
  print OUT "#\n";
  print OUT "# created by $0\n";
  print OUT "#\n";
  print OUT "# lang=<LANGUAGE>         sets language (<LANGUAGE> = one of de,en,fr,...) for all subsequent texts\n";
  print OUT "# enum=<ENUM>             name of enum to use for all subsequent texts. Allows to define several enums in one file\n";
  print OUT "# <ENUM>   <LOCALE_TEXT>  defines one locale text in current language (<ENUM> = name of enum-constant)\n";
  print OUT "#\n";
  print OUT "# Example:\n";
  print OUT "# enum=log_texts_e\n";
  print OUT "# lang=en\n";
  print OUT "# LOG_TEXT_HELLO_WORLD  Hello world\n";
  print OUT "# lang=de\n";
  print OUT "# LOG_TEXT_HELLO_WORLD  Hallo Welt\n";
  print OUT "# lang=fr\n";
  print OUT "# LOG_TEXT_HELLO_WORLD  Bonjour monde\n";
  print OUT "#\n";
  close(OUT);
}

#$Name .= "_test";
createLanguages("languages.h");
createHeaderFile($Name.".h", \%Languages);
createCFile($Name.".c", $Name.".h", \%Languages);
createHeaderFile($Name."_cpp.h", \%Languages);
createCppFile($Name."_cpp.c", $Name."_cpp.h", \%Languages);

sub createLanguages {
  my $Source = shift;
  my $SOURCE = uc($Source); $SOURCE =~ s/\./_/g;

  my $Languages = join(",\n    ", @Languages);

  print "creating file '$Folder$Source'\n";
  open(SOURCE, ">$Folder$Source") or die("ERROR: Cannot create file '$Source' ($!)"); #{
    print SOURCE <<"END_OF_TEXT";
/* $Source
 *
 * Note: This file has been automatically generated from $FileName by $0
 *       Any changes made to this file will get lost!
 *
 * This file may be included from C++ and from C source.
 * Text encoded as UTF-8.
 */
 
#ifdef __cplusplus
 extern "C" {
#endif

#ifndef $SOURCE
#define $SOURCE


typedef enum Language_e {
    $Languages
} LANGUAGE_E;

#endif //$SOURCE

#ifdef __cplusplus
 }
#endif

END_OF_TEXT
  
  close(SOURCE);
  #}

}
sub createCppFile {                                                                                                                        
  my $Source       = shift;
  my $Header       = shift;
  my $LanguagesRef = shift;
  my $HEADER = uc($Header); $HEADER =~ s/\./_/g;
  
  my @Strings;
  my %AllEnums;
  my $Table;
  my $TableRows;
  my $Functions;
  
  foreach my $Language (sort keys %$LanguagesRef) {
    my $TextsRef = $LanguagesRef->{$Language}->{Entries};
    my @TextNames = grep { strip($_) } @{ $LanguagesRef->{$Language}->{TextNames} };
    foreach my $TextName (@TextNames) {
      my $EntryRef = $TextsRef->{$TextName};
      my $EnumName = $EntryRef->{Enum};
      unless ( defined($AllEnums{$EnumName}->{$Language}->{Entries}->{$TextName}) ) {
        $AllEnums{$EnumName}->{$Language}->{Entries}->{$TextName} = [];
        push @{ $AllEnums{$EnumName}->{$Language}->{TextNames} }, $TextName; 
      }
      $AllEnums{$EnumName}->{$Language}->{Entries}->{$TextName} = $EntryRef;
    }
  }
    
  foreach my $EnumName (sort keys %AllEnums) {
    my $TextCases;
    my $Index = 0;
    my $LanguageCases;
    my $EnumType = $EnumName.'_e';
	my $ENUMTYPE = uc($EnumType); $ENUMTYPE =~ s/\./_/g;

    foreach my $Language (sort keys %{ $AllEnums{$EnumName} }) {
      my @AllTextNames = grep { strip($_) } @{ $AllEnums{$EnumName}->{$Language}->{TextNames} },
                         grep { strip($_) } @{ $AllEnums{$EnumName}->{en}->{TextNames} };
      my %TextNames;
      my @TextNames;
      foreach my $TextName (@AllTextNames) { # remove doubles
        unless ($TextNames{$TextName}) {
          $TextNames{$TextName} = 1;
          push @TextNames, $TextName;
        }
      }
      
      
      if (@TextNames) {
        my @Rows;
        foreach my $TextName (@TextNames) {
          my $LocalTextName = $TextName; #.'_'.$Language;
          my $EntryRef = $AllEnums{$EnumName}->{$Language}->{Entries}->{$TextName};
          my $String = $EntryRef->{String};
          unless ($String) { $String = $AllEnums{$EnumName}->{en}->{Entries}->{$TextName}->{String}; }
          push @Rows, "case $LocalTextName: return \"".$EntryRef->{String}."\";";
        }
        $TextCases = join("\n        ", @Rows);
        $LanguageCases .= <<"END_OF_TEXT";
    case $Language: {
      switch (Text) {
        $TextCases
        default: return "!TEXT!";
      }
    }
END_OF_TEXT
      }
    }
  $Functions .= <<"END_OF_TEXT";
const char *getLocalText_$EnumName\($ENUMTYPE Text, LANGUAGE_E Language) {
  switch (Language) {
$LanguageCases
    default: return "!LANGUAGE!";
  }
}
END_OF_TEXT
  }
  print "creating file '$Folder$Source'\n";
  open(SOURCE, ">$Folder$Source") or die("ERROR: Cannot create file '$Source' ($!)"); #{
    print SOURCE <<"END_OF_TEXT";
/* $Source
 *
 * Note: This file has been automatically generated from $FileName by $0
 *       Any changes made to this file will get lost!
 *
 * This file is especially formatted to be included from C++ source.
 * Text encoded as UTF-8.
 */
 
#include "$Header"

$Functions
END_OF_TEXT
  
  close(SOURCE);
  #}
}
sub createCFile {                                                                                                                          
  my $Source       = shift;
  my $Header       = shift;
  my $LanguagesRef = shift;
  my $HEADER = uc($Header); $HEADER =~ s/\./_/g;
  
  my @Strings;
  my %AllEnums;
  my $Table;
  my $TableRows;
  
  my $Language = "en";
  my $TextsRef = $LanguagesRef->{$Language}->{Entries};
  my @TextNames = grep { strip($_) } @{ $LanguagesRef->{$Language}->{TextNames} };
  foreach my $TextName (@TextNames) {
    my $EntryRef = $TextsRef->{$TextName};
    my $EnumName = $EntryRef->{Enum};
    unless ( defined($AllEnums{$EnumName}->{Entries}->{$TextName}) ) {
      $AllEnums{$EnumName}->{Entries}->{$TextName} = [];
    push @{ $AllEnums{$EnumName}->{TextNames} }, $TextName; 
    }
    $AllEnums{$EnumName}->{Entries}->{$TextName} = $EntryRef;
  }
  
  my $TextsAndArrays;
  foreach my $EnumName (sort keys %AllEnums) {
    my $Texts;
    my $Array;
    my $Index = 0;
    my $EnumNameUC = uc($EnumName)."_COUNT";
    my $EnumType = $EnumName.'_e';
	  my $ENUMTYPE = uc($EnumType); $ENUMTYPE =~ s/\./_/g;
    my @TextNames = grep { strip($_) } @{ $AllEnums{$EnumName}->{TextNames} };
    if (@TextNames) {
      my @Rows;
      foreach my $TextName (@TextNames) {
        my $LocalTextName = $TextName; #.'_'.$Language;
        my $EntryRef = $AllEnums{$EnumName}->{Entries}->{$TextName};
        $Texts .= 'const char text_'.$LocalTextName.'[] = "'.$EntryRef->{String}."\";\n";

        push @Rows, '  /* '.($Index++)." */    text_$LocalTextName";
      }
      $Array = "const char* const p_".$EnumName."[] = \n{\n".join(",\n", @Rows)."\n};\n";
    }
    $TextsAndArrays .= <<"END_OF_TEXT"; #{
$Texts

$Array

const char *getLocalText_$EnumName\($ENUMTYPE Text, LANGUAGE_E Language)
{
  if(Text >= $EnumNameUC)
  {
	return p_$EnumName\[0];
  }
  Language = Language;
  return p_$EnumName\[Text];
}

END_OF_TEXT
#}
  }
  print "creating file '$Folder$Source'\n";
  open(SOURCE, ">$Folder$Source") or die("ERROR: Cannot create file '$Source' ($!)"); #{
    print SOURCE <<"END_OF_TEXT";
/* $Source
 *
 * Note: This file has been automatically generated from $FileName by $0
 *       Any changes made to this file will get lost!
 *
 * This file is especially formatted to be included from C source.
 * Text encoded as UTF-8.
 */

#include "$Header"

$TextsAndArrays
END_OF_TEXT
  
  close(SOURCE);
  #}
}
sub createHeaderFile {                                                                                                                     
  my $Header       = shift;
  my $LanguagesRef = shift;
  my $HEADER = uc($Header); $HEADER =~ s/\./_/g;
  
  my @Strings;
  my %AllEnums;
  my $Table;
  my $TableRows;
  
  foreach my $Language (@Languages) {
    
    my $TextsRef = $LanguagesRef->{$Language}->{Entries};
    my @TextNames = grep { strip($_) } @{ $LanguagesRef->{$Language}->{TextNames} };
    foreach my $TextName (@TextNames ) {
      my $EntryRef = $TextsRef->{$TextName};
      my $EnumName = $EntryRef->{Enum};
      unless ( defined($AllEnums{$EnumName}->{Entries}->{$TextName}) ) {
        $AllEnums{$EnumName}->{Entries}->{$TextName} = [];
        push @{ $AllEnums{$EnumName}->{TextNames} }, $TextName; 
      }
      $AllEnums{$EnumName}->{Entries}->{$TextName} = 1;
    }
  }
  my $Enums;
  foreach my $EnumName (sort keys %AllEnums) {
    my @TextNames = grep { strip($_) } @{ $AllEnums{$EnumName}->{TextNames} };
    if (@TextNames) {
      my $EnumType = $EnumName.'_e';
	  my $ENUMTYPE = uc($EnumType); $ENUMTYPE =~ s/\./_/g;
      my $ENUMTYPE = uc($EnumType);
      $Enums .= "typedef enum $EnumType {\n    ".
                join(",\n    ", @TextNames).",\n".
				"    ".uc($EnumName)."_COUNT\n".	
                "} $ENUMTYPE;\n\n".
                "const char *getLocalText_$EnumName\($ENUMTYPE Text, LANGUAGE_E Language);\n\n";
    }
  }
  
  my $LanguageEnums = "typedef enum Language_e {\n  ".
                      join(",\n  ", @Languages)."\n".
                      "};\n";

  print "creating file '$Folder$Header'\n";
  open(HEADER, ">$Folder$Header") or die("ERROR: Cannot create file '$Header' ($!)"); #{
    print HEADER <<"END_OF_TEXT";
/* $Header
 *
 * Note: This file has been automatically generated from $FileName by $0
 *       Any changes made to this file will get lost!
 * Text encoded as UTF-8.
 */
 
#ifdef __cplusplus
 extern "C" {
#endif

#ifndef $HEADER
#define $HEADER

#include "languages.h"

$Enums

#endif //$HEADER

#ifdef __cplusplus
 }
#endif


END_OF_TEXT
  
  close(HEADER);
  #}

}
sub strip {                                                                                                                                
  my @Stripped;
  
  my %IsWhiteChar = ( " " => 1, "\t" => 1, "\n" => 1 );
  foreach my $String (@_) {
    while ( $IsWhiteChar{ substr($String, 0, 1) } ) {
      substr($String, 0, 1) = undef;
    }
    while ( $IsWhiteChar{ substr($String, -1, 1) } ) {
      substr($String, -1, 1) = undef;
    }
    push @Stripped, $String;
  }
  
  if (wantarray) { return @Stripped; }
  else           { return join(" ", @Stripped); }
}
