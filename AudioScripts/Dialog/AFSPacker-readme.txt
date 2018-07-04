      #----------------------------------------------------------------#
      #                    AFS Packer - Version 1.1                    #
      #           By PacoChan - http://pacochan.tales-tra.com          #
      #----------------------------------------------------------------#

AFS Packer can extract the files of an AFS archive to a folder, or
generate a new AFS archive with the files inside a folder. The AFS format
is used in many games. Many of them from Sega.

The program requires .NET Framework 3.5.

Usage:

  AFSPacker -e input_file ouput_dir [list_file]   :  Extract AFS file

  AFSPacker -c input_dir output_file [list_file]  :  Create AFS file

    list_file: will create or read a text file containing a list of all
               the files that will be extracted/imported from/to the AFS file.
               This is useful if you need the files to be in the same
               order as in the original AFS (for example: Shenmue 1 & 2).


Changelog:

	V1.1: Fixed a crash reading AFS files in games like Arc Rise Fantasia.
	V1.0: Initial release.