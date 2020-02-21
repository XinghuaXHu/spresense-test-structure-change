#! /usr/bin/env perl

# �\�[�X�t�@�C����
$cpp_name = "audio_manager.cpp";

# �w�b�_�[�t�@�C����
$hd_name = "audio_manager.h";

# �������[�h
$word_src2 = "void AudioManager::run";
$word_src3 = "while";
$word_src4 = "private";

# �t�@�C�����I�[�v��
open(FH_SRC, "../../../../../audio/manager/audio_manager.cpp");
open(FH_DST, "> ../src/audio_manager.cpp");

$find_run = 0;
$chg_while = 0;
while (my $line = <FH_SRC>) {
	if($find_run == 0){
		if ($line =~ /$word_src2/){
			$find_run =1;;
		}
		print FH_DST $line;
	}
	elsif($chg_while == 0){
		if($line =~ /$word_src3/){
			$line2 = "	{\n";
			$chg_while = 1;
			print FH_DST $line2;
		}
		else{
			print FH_DST $line;
		}
	}
	else{
		print FH_DST $line;
	}
}
close(FH_SRC);
close(FH_DST);

# �t�@�C�����I�[�v��
open(FH_SRC, "../../../../../audio/manager/audio_manager.h");
open(FH_DST, "> ../include/audio_manager.h");
$comment = "//";

while (my $line = <FH_SRC>) {
	if ($line =~ /$word_src4/){
		$new_line = $comment . $line;
		print FH_DST $new_line
	}
	else{
		print FH_DST $line;
	}
}
close(FH_SRC);
close(FH_DST);
