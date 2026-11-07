#!/usr/bin/env perl
#
# Copyright 2016 Ken Brown.
# This file is licensed under the GNU General Public License version 2
# or any later version.
#
# This is intended to be run from a postinstall or preremove script.
#
# Usage: update_tlpdb.pl ACTION [PKG]...
# ACTION is add or remove.

our $Master;

BEGIN {
  $^W = 1;
  $Master = "/usr/share";
  $::installerdir = $Master;
  unshift (@INC, "$Master/tlpkg");
}

use strict;

use TeXLive::TLPDB;
use TeXLive::TLPOBJ;
use TeXLive::TLUtils;
use TeXLive::TLConfig;

my $prg = TeXLive::TLUtils::basename($0);

my $action = shift @ARGV;

usage() if !defined($action);

$action = lc($action);

usage() if $action !~ /^(add|remove)$/;

my $dbfile = "$Master/tlpkg/texlive.tlpdb";
my $tlpdb = TeXLive::TLPDB->new;

if (-e $dbfile) {
  $tlpdb->from_file($dbfile);
} else {
  initialize();  
}

my $ret;
if ($action eq "add") {
  # We want to add pkg before pkg.doc.
  $ret = do_add(sort @ARGV);
} elsif ($action eq "remove") {
  $ret = do_remove(sort @ARGV);
}

exit ($ret);

####################################

# Adapted from the action_include_tlpobj function in tlmgr.pl.  If
# we’re adding an ordinary package and the corresponding doc package,
# the ordinary package must be done first.
sub do_add {
  my %added;
  for my $f (@_) {
    my $tlp = TeXLive::TLPOBJ->new;
    $tlp->from_file("$Master/tlpkg/tlpobj/$f.tlpobj");
    my $pkg = $tlp->name;
    if ($pkg =~ m/^(.*)\.doc$/) {
      # doc package; maybe we’ve already taken care of it.
      my $mothership = $1;
      next if $added{$mothership};
      my $mothertlp = $tlpdb->get_package($mothership);
      if (!defined($mothertlp)) {
        # The best we can do is rename the package to $mothername and add it.
        $tlp->name($mothership);
        $tlpdb->add_tlpobj($tlp);
      } else {
        $mothertlp->docfiles($tlp->docfiles);
        $mothertlp->docsize($tlp->docsize);
        $tlpdb->add_tlpobj($mothertlp);
      }
    } else {
      # Ordinary package; add docfiles if we’ve got them.
      if (-e "$Master/tlpkg/tlpobj/$f.doc.tlpobj") {
        my $doctlp = TeXLive::TLPOBJ->new;
        $doctlp->from_file("$Master/tlpkg/tlpobj/$f.doc.tlpobj");
        $tlp->docfiles($doctlp->docfiles);
        $tlp->docsize($doctlp->docsize);
      }
      $tlpdb->add_tlpobj($tlp);
      $added{$pkg} = 1;
    }
  }
  $tlpdb->save;
  # We can't do proper error-checking because tlpdb->save doesn't.
  return $F_OK;
}

sub do_remove {
  my %removed;
  for my $f (@_) {
    my $tlp = TeXLive::TLPOBJ->new;
    $tlp->from_file("$Master/tlpkg/tlpobj/$f.tlpobj");
    my $pkg = $tlp->name;
    if ($pkg =~ m/^(.*)\.doc$/) {
      my $mothership = $1;
      my $mothertlp = $tlpdb->get_package($mothership);
      next if !defined($mothertlp);
      $mothertlp->clear_docfiles;
      $tlpdb->add_tlpobj($mothertlp);
    } else {
      $tlpdb->remove_tlpobj($pkg);
    }
  }
  $tlpdb->save;
  # We can't do proper error-checking because tlpdb->save doesn't.
  return $F_OK;
}

sub initialize {
  $tlpdb->root($Master);
  my $inst = TeXLive::TLPOBJ->new;
  $inst->name("00texlive.installation");
  $inst->category("TLCore");
  my $arch = TeXLive::TLUtils::platform();
  my @deps = ( "setting_available_architectures:$arch" );
  $inst->depends(@deps);
  $tlpdb->add_tlpobj($inst);
  # $tlpdb->add_default_options();
  $tlpdb->option("location", "http://mirror.ctan.org/systems/texlive/tlnet");
  $tlpdb->option("install_docfiles", 1);
  $tlpdb->option("install_srcfiles", 1);
  $tlpdb->save;
}

sub usage {
  tlwarn("Usage: $prg add|remove [PKG]...\n");
  exit($F_ERROR);
}


### Local Variables:
### perl-indent-level: 2
### tab-width: 2
### indent-tabs-mode: nil
### End:
