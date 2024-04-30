#! /usr/bin/env perl

use strict;
use warnings;

use Cwd qw( cwd getcwd realpath );
use Getopt::Long;
use File::Temp qw( tempdir );

my $arg = 0;
my @filelist = [];
my $use_indent = "";
my $mvapich_files = "";
my $all_mvapich_files = "";
my $git_diff = "";
my $diff_branch = "";
my $mpich_repo = "https://github.com/pmodels/mpich.git";
my $mpich_base = "v3.4.3";
my $mpich_branch = "3.4.x";
my $format = "";
my $check = "";

sub usage
{
    print "Usage: $0 [OPTIONS] [filelist]\n\n";
    print "OPTIONS:\n";

    print "\t--git-diff             Format the git diff. By default this is the unstaged changes\n";
    print "\t--branch=base-branch   Format the git diff from this branch (optional)\n";
    print "\t--mvapich-files        Format the touched mvapich files\n";
    print "\t--all-mvapich-files    Format all mvapich files\n";
    print "\t--use-indent           Format using GNU indent (default is clang-format)\n";
    print "\t--check                Do not reformat, just emit errors where formating is wrong\n";

    print "\t--help                 Show this help text\n";
    print "\n";
    exit 1;
}

sub get_files
{
    my $file = "";
    foreach $file (@ARGV) {
        unless (-f $file) {
            print "$0 expects a list of files\n";
            print "$file does not exist\n";
            exit 1;
        }
    }
    @filelist = @ARGV;
    print "Formatting @filelist\n";
}

sub get_mvapich_files
{
    my @c_files = "";
    unless ($all_mvapich_files) {
        my @touched_files = `git diff master --stat | cut -d' ' -f 2 | head -n -1`;
        @c_files = grep m/\.(c|cpp|cu|h)$/, @touched_files;
    } else {
        my $branch = `git branch --show-current`;
        my @all_files = `git ls-tree --full-tree -r --name-only $branch`; #| grep -E '\\\.(c|cpp|cu|h|y|l)\$'`;
        @c_files = grep m/\.(c|cpp|cu|h)$/, @all_files;
    }
    my @base_files = `git ls-tree --full-tree -r --name-only $mpich_base`;
    my %base_hash;
    @base_hash{@base_files} = ();
    my @mvapich_files = grep ! exists $base_hash{$_}, @c_files;
    @filelist = @mvapich_files;
    print "Formatting @filelist";
}

sub check_format_method
{
    if (!$use_indent and (`which clang-format` eq "")) {
        print "clang-format preferred but not found\n";
        print "Please specify --use-indent or add clang-format to your path\n";
        exit 1;
    } elsif ($use_indent and (`which indent` eq "")) {
        print "indent specified but not found\n";
        print "Please try without --use-indent or add indent to your path";
        exit 1;
    } elsif ($use_indent) {
        my $v = `indent --version | head -1 | cut -f3 -d' ' | xargs echo -n`;
        print "indent specified and version $v found, using indent for formatting\n";
        print "Note that clang-format is prefered in favor of indent\n";
        $format = "indent";
    } else {
        my $v = `clang-format --version | head -1 | cut -f3 -d' ' | xargs echo -n`;
        print "clang-format version $v found, using clang-format for formatting\n";
        $format = "clang-format";
    }
}

sub format_diff
{
    # process checking 
    if ($use_indent) {
        print "Diff based format is only available with clang-format.\n";
        exit 1;
    }
    # if a branch was specified, format the diff from that branch
    my $branch = $diff_branch;
    my @error_list;
    #unless (`which git-clang-format` eq "") {
    unless ((`which git-clang-format` eq "")) {
        # easy mode - the git script exists
        #my @files = `git ls-files -m`;
        #my @c_files = grep m /\.(c|cpp|cu|h)$/, @files;
        my $command = "git clang-format --style=file --extensions=c,h,cpp,cu";
        if ($branch) {
            $command .= " $branch";
        }
        if ($check) {
            $command .= " --diff -v";
            #my $rc = system("$command @c_files | grep -e \"no modified files\" -e \"did not modify\"");
            print "$command\n";
            not system("$command | grep -e \"no modified files\" -e \"did not modify\"")
                or die("Formatting errors found\nPlease use this script to reformat your code\n");
            print "No formatting errors found\n";
        } else {
            #system($command @c_files);
            system($command);
            exit 0;
        }
    } else {
        # expirimental mode - my version 
        # get the list of edited files
        unless ($check) {
            print "Using the manual git diff process. Please note this may result ";
            print "in errors as I have not yet worked out all the bugs. ";
            print "Be sure to double check the results before commiting. If you ";
            print "can install the gitlab-clang-format extension that is the ";
            print "prefered method.\n";
        }
        my @diff = "";
        if ($branch) {
            @diff = `git diff $branch`;
        } else {
            @diff = `git diff`;
        }
        my $diff_file = "";
        my $start = 0;
        my $range = 0;
        my $skip = 0;
        foreach my $line (@diff) {
            # extract file names from the diff
            if ($line =~ /^\+\+\+\s+[a|b]\/(\S+)/gm) {
                unless ($line =~ m/\.(c|cpp|cu|h)$/) {
                    $skip = 1;
                } else {
                    $skip = 0;
                    $diff_file = $1;
                    $range = 0;
                }
            } elsif ($line =~ /^@@\s+\-(\d+),(\d+)\s+\+(\d+),(\d+)\s+@@/gm and !$skip) {
                # match diff hunk - grab the third group for the start line
                # grab the fourth group for the range of the diff
                $start = $3;
                $range = $4;
                my $end = $start + $range;
                my $command = "$format -i -lines=$start:$end";
                if ($check) {
                    $command .= " --dry-run -Werror";
                }
                my $res = system("$command $diff_file");
                # add this file to the error list and skip the rest
                if ($check and $res) {
                    push(@error_list, $diff_file);
                    $skip = 1;
                }
            }
        }
        if ($check and (scalar(@error_list) != 0)) {
            print "Formatting errors in these files: \n@error_list";
            print "Please use this script to reformat your code\n";
            exit 1;
        } elsif ($check) {
            print "No formatting errors found\n";
            exit 0;
        }
    }
}

sub check_remote
{
    my $remote = `git remote -v`;
#   unless ($remote =~ /upstream\s+${mpich_repo}\s+\(fetch\)/ and
#           $remote =~ /upstream\s+${mpich_repo}\s+\(fetch\)/) {
    unless ($remote =~ /${mpich_repo}\s+\(fetch\)/ and
            $remote =~ /${mpich_repo}\s+\(fetch\)/) {
        print "Repo not setup to push/fetch from upstream MPICH\n";
        system("git remote add -m ${mpich_branch} upstream ${mpich_repo}");
        system("git fetch --tags upstream"); 
        system("git remote -v");
    } else {
        print "Repo already configured to push/fetch from upstream MPICH\n";
        system("git remote -v");
    }
}


GetOptions(
    "git-diff" => \$git_diff,
    "branch=s" => \$diff_branch,
    "mvapich-files" => \$mvapich_files,
    "all-mvapich-files" => \$all_mvapich_files,
    "use-indent" => \$use_indent,
    "check" => \$check,
    "help" => \&usage,
) or die "unable to parse options, stopped";

# option checking
if ($git_diff and ($mvapich_files or $all_mvapich_files)) {
    print "--git-diff cannot be used with --mvapich-files. Please select one.\n";
    exit 1;
} elsif ($all_mvapich_files and !$mvapich_files) {
    print "--all_mvapich_files is not valid without selecting --mvapich-files\n";
    exit 1;
}
# a list of files was provided
if ((scalar(@ARGV) != 0) and ($mvapich_files or $git_diff)) {
    print "Cannot provide file list and --mvapich-files or --git-diff at the same time\n";
    print "Please use one of the automatic file selection policies or the filelist\n";
    exit 1;
} elsif ((scalar(@ARGV) == 0) and !($mvapich_files or $git_diff)) {
    print "No options provided, please provide one of the file selection criteria: \n";
    print "--mvapich-files, --git-diff, or a file list\n";
    exit 1;
}

# check for indent/clang-format
check_format_method();
print("\n");

if (scalar(@ARGV) != 0) {
    get_files();
    print("\n");
} elsif ($mvapich_files) {
    check_remote();
    get_mvapich_files();
}
chomp(@filelist);

if ($git_diff) {
    format_diff();
} elsif ($use_indent) {
    print "indent formatting not yet supported\n"
    #format_indent();
} else {
    my $command = "clang-format -style=file";
    if ($check) {
        $command .= " -dry-run -Werror";
    } else {
        $command .= " -i"
    }
    my $res = system("$command @filelist");
    if ($check and $res) {
        print "Formatting errors found, please use this script to reformat your code\n";
        exit 1;
    } elsif ($check) {
        print "No formatting errors found.\n"
    }
    exit 0
}
