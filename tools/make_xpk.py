#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Generate XPK package from package resources and the author private key.
"""
from Crypto.PublicKey import RSA
from Crypto import Random
from Crypto.Signature import PKCS1_v1_5
from Crypto.Hash import SHA
import argparse
import json
import os
import struct
import traceback
import zipfile

class XPKGenerator(object):
  def __init__(self, source_dir, key_file, output_file):
    """
    source_dir  : the path to package resource directory.
    key_file    : the path to RSA private key file, if the file is invalid,
                  generator will create it automatically.
    output_file : the output XPK file path.
    """
    self.source_dir_ = source_dir
    self.output_file_ = output_file
    if not os.path.exists(key_file):
      try:
        print('Start to generate RSA key')
        rng = Random.new().read
        self.RSAkey = RSA.generate(1024, rng)
        kfile = open(key_file,'w')
        kfile.write(self.RSAkey.exportKey('PEM'))
        kfile.close()
        print('Finished generating RSA key, saved as %s' % key_file)
      except IOError:
        if os.path.exists(key_file):
          os.remove(key_file)
        traceback.print_exc()
    else:
      self.RSAkey = RSA.importKey(open(key_file, 'r').read())
    self.pubkey = self.RSAkey.publickey().exportKey('DER')

  def Generate(self):
    if not os.path.exists(self.source_dir_):
      print("The source directory %s is invalid." % self.source_dir_)
      return
    try:
      zip_file = '%s.tmp' % self.output_file_
      self.__SetIconInManifest(self.source_dir_)
      self.__Compress(self.source_dir_, zip_file)
      signer = PKCS1_v1_5.new(self.RSAkey)
      zfile = open(zip_file, 'rb')
      sha = SHA.new(zfile.read())
      signature = signer.sign(sha)
      xpk = open(self.output_file_, 'wb')
      zfile.seek(0)
      print('Generating XPK package: %s' % self.output_file_)
      xpk.write('\x43\x72\x57\x6B')
      xpk.write(struct.pack('<I', len(self.pubkey)))
      xpk.write(struct.pack('<I', len(signature)))
      xpk.write(self.pubkey)
      xpk.write(signature)
      xpk.write(zfile.read())
      zfile.close()
      xpk.close()
      print('Generated new XPK package %s successfully.'
            % self.output_file_)
    except IOError:
      if os.path.exists(self.output_file_):
        os.remove(self.output_file_)
      traceback.print_exc()
    finally:
      if os.path.exists(zip_file):
        os.remove(zip_file)

  @classmethod
  def __SetIconInManifest(cls, source_dir):
    image_str = 'iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAABmJLR0QA/wD/' \
                'AP+gvaeTAAAACXBIWXMAAABIAAAASABGyWs+AAAwLUlEQVR42u2deZxcV3Xn' \
                'v+e+Wrq6W+rWLmtrLd5tbMtgG2NjLEMICdtAMpN8khAIGCaTQGyYZIAA+fAB' \
                'PpOQBGxwmAnBCQQSMplMEkKAQBKwEDZeAGuxsa1dau1St3qvrqpX797549y3' \
                'VKtVvUty0Pn4udVVr7rOu79zz37vFZ6HtO/X7hj7kgBtQCtQAjqBDcBaoAtY' \
                'BSwBFvqrFcj5nwBloO5/nvbXKeAwcBA4AOwF+oFRf98I4LJMrP/8lvM9NFMm' \
                'Od8MTJbGAb0VBXURCvQNwAuAK4DVQAtg/DNO9zmdvyxQAQ4BO4GngG2oYPSi' \
                'wlLOfvD5IgwXvACMAb4NndkbUMBvBq4HLhnno1VgABjyVxkFsYLOdut/gmoD' \
                '43+2+KsVmOev+f61sXQM2A48gQrEXn+NxDdc6IJwwQrAGOCvAm4EXgjcioI/' \
                'FpBYXR9FgTkCHEdnZw+qvmNhqAIRqQoXIACKpKB3AotRLbMMNSPLgZWkZiVL' \
                'FVQIHgN+CDwJPBu/eaEKwgUnABng24DbgTuBF6Ogd2Zu7QOeRmfgLlQdH0SB' \
                '750j9haRCsBa4HJUA10LLMjc108qDJuBh/Fa4UIThAtGADLALwJeCbwenfUb' \
                'UPUMMAg8DnwHBf4gOvMHzxPb81FN0IUKwl3ALf51UDOzF9UG/wT8K144LxRB' \
                'OG8CMEbFG6ADeB3wy6gzt9y/VwN2A38PPITO9GOoGh/7LCZzLQXWowCtRNV4' \
                'JzpTS6i6L2S+o4p6+H3oDD6BapPDwD7gJApofLkx319EfZEuVBB+Drgs8x3H' \
                'Uefxr4Gvov6JjT98vgTivAhABnxBgb8L+HXgRShAFp3VjwB/g6rQHjLOlacc' \
                'OvCtqDp+EbARuBq13S0oAHl/5VBbf7bndqhvUAdCf9VQ+34KtelPojZ+F+pY' \
                'VkmdyZjaUP/hNuCX/M/5qGD2+c9/Fvg2KggOzo8QnHMByIDfjtrOdwGv8QNk' \
                '0YF+CPhzP1DDNA5wDhWaZai63YQ6hktpBHouKCsYJ4FHPa+PoxpjYBxe21DB' \
                'vNvzugQVhEHga8ADqC8zDOdeCM6pAHjwBVXNbwLejqpNQdX6wyjw30PVcVbN' \
                'dqDq9YXAq1DncAE6o81kvn8OyKIaow919r4J/Aj1TQYy9wlqdm5HBeF2/9zO' \
                'P/fngC+hpsadSyE4JwKw761+1jvmAz8F/CbwUnSGDKCAfwn4OjCScCWAZTUa' \
                'AWwCXo3a1QvGeR1DDvVXvo5qhm0YDiVirD/b/HO8CRWETlRrfA/4DPBviDq1' \
                '6/9i7gVhzgcyAV+9+btxvIXUwXsctfH/F50JWY42oLP8VTjuQlO4zyc6jfAd' \
                'VCtsRqOBrE67BPjPqI9wi3/tOMIXgAfj++daCOZMADLAB6iT927g5ahT1gP8' \
                'H+CL1PgB+QZOVqIh4GvRGdI+pyMw9zSMmravoaHgYUAFIQQK3AT8KvCLqONY' \
                'A/4duB8NdyOYO0GYK2cpphbg11BH7yr/2g+BT6Fqsi8JkhTo16Kq8VYakz7P' \
                'Z2pHfZYXo87ul4CvIgz7Z/8BsAfVhvegDuPPAuuATwNfQKOQOaFZ1wB+5gua' \
                '0Hk/8Cuoh14G/gG4D9hB6i3n0dDtXtTOL5/SFz7/6DjqH9wPbEX1AKimvA54' \
                'D/BGNLQ9geYN/gDVmm62NcGsCkAG/C7gY8AbMg/yv1Enpyfz3YuAt6A5gC7m' \
                'XiNdKFRHI4U/RWd4L6l3sBh1kv8bGuqWgX8EPug/M6tCEMzWH/LgGzSL9wnU' \
                'jheB5zzznyNN2RbQNO8ngLf6Bz1fodz5IIM6tbehKeTd6MSIUMAfB7qBa1Bn' \
                '8Wq0zP00cOqejV3uU1sPzgojsyIAGfBfCHwc+Gl0hj+OqrRvoM4NaOz+X1Dw' \
                'b0WF5CeVCsClwMvQyXGQtFz9LAr4FWg6+3J/77PA8dkSghkLQAb8W4DfRz19' \
                'h4Y/70GFIM55d/nX3oc2bVyo8fy5JEHV/kvRHMEu0jrBQdRp7kLBj6+dwNHZ' \
                'EIIZCUDG5r8IdVQ2odL7FeC9wI8zt18PfBR4M8//0G4uqATcBKxBTcJx//px' \
                'NLu4AtUCcUPMj4Fj92zsYiZCMG0B2P/WO3Agop7rx9FYPwL+H/ABYLcADozA' \
                'HaiAvHom3/kTQAFq968EDjroNqpNe9Ei1HL/3no0TNzu4OS7ZyAEM3K8DKwR' \
                '+H2BVwhEojP/Q2hcCxAY+BmB+wXujJvzLl4TXncK3GfgZ0gnzB7gQwL/5Mf6' \
                'FQL/06jGmDZNazbuV9W/GJ35r/cv/yvwXge7fCdmzr/3h2jV7yJNjZajTvVR' \
                'UW1qLZw2mkOJzcB6tLr4vXs3dpWnowWmLAAe/BLwYTRrV0Adlf8OPOW9uhj8' \
                'j6N26yJNjxaj4fJhYLdRx7AHDa1vRKODy9Bcy5Z7N3bVpyoEUxIAD34eLeO+' \
                'E03X7kQ9+8dQe5VDU58f58Ku3D1faAFaDd3nL4sm1vaikdclqDYYArbeu7HL' \
                'TkUIpuMDbAJ+A03vnkA9+y2eMYMWcD6KzvyL4M+cBB3Lj6Jja9Cx3uJfO4Fi' \
                '8RsoNlOiSWuA/W+7A9TmfARN4JTRos6DQA0DOG5AZ/4tXAR/NklQn2A9sI2A' \
                '4zgsGi4W0PFegZqMR++9satvslpgUhrAg9+J8A403AMt7HwGqKDx3hqE30VD' \
                'vos0N3QHwgewrPHTqwL8LxQLgLs8Rp0eswlpsiYgrun/KkIR4YcInyRPj2ek' \
                'E/UJXnu+46f/8JeWlNX/EiBHD8InEX6AUER7C+5iktp9QgHwkrQWrelfgjZt' \
                'fgp42hd0i2jV7238ZOf1zxUV0bF+A1DUdhGeRjE5iWL0LmDtZLRAUwHwf6Cd' \
                'tH/NAX8L/DNaxxY0E/g+nn8tW89nWoiO+XUoBiHacfS3KEa3o5i1TyQEZxUA' \
                '/0FBkzjvQMO7J9COlrjjdRGa+bvsfI/ITyBdBvweigEoJl9CMcqhmF0HSDMh' \
                'mMgEzEfVyXK0XPk3aGECNB/wFtTeXPT4zz0JGva9BcUCtF7wZVQYlqMtZh3N' \
                '/kgzARAU3NegauV7aKEn8u9vRDt52s73SPwEUxuKwUb/e1yMexjF7FWokJx1' \
                'gjYTgE7/x+ehzsUXgSPr/nwL/rV7mGEh4iLNCq1BsZjnsTkK/CWaIJqHYrjg' \
                'bB8eVwD2330HCK9DN2CI0Nn/9XV/viX73itIVc9FOn+UB16B8Lr9d9+BF4Kv' \
                'o1ogAm5BeO3+u8f3A86mARYCv4wwH6EX4UHShZmr/HtLzntMfPGKryXoqup4' \
                '04oywoMeu3n+vXGjtDMEwEvKK1EP0uE3OEhmvy7hfgkXHb8LiQTFJKsFHkbb' \
                'zx2K5U+PpwXG0wDz0CTDYnRVy18Ao/7DG9DFG009y4t0XqgDxWaDx2oUxW4Y' \
                'xfINKLYN1CAA/oMvQWvNAbr8+QlUigzavXr7+X7Si3RWuh3FSEtzuuroURTL' \
                'jcBLxmqBsRogB9yJVp2qaGJh2L+3Em1RutjQeeFSO4rRSv/7MBq9VVFM72TM' \
                '4puxAnAZuobNoI0eDwN1HEK6RPsiXdi0CbjBY1ZHd1nZiWJ6K3D5/renWiAR' \
                'AP/ijWj7NuhypJ51D24Bw3z/hxc1fJWz2qnmL5e9nF44ywVBzuLqtYbLho2X' \
                'q4c4G838u2aL5Shsyq8Na7ho7O40LAI2YZi/7sEtoC1kcbn4OhTjhLLqoB1J' \
                '9ugZAB6iQNkLxhqEVzd8jQi5xauQQuN2fcnyd6f/ctUK0enjye/nZyQd+ZXr' \
                'KF1z8/i8xr+HNWqH91Lt3oWrVRGR8xfqiND+olcSLFh8Vn5xEPWfYuTJ7+Kc' \
                'y1SMeTXwhf1vv2MHecqEbEYxXYCuPfgK3rRnBWA9qv5BV/Mc8Iu58ujCjzMK' \
                'PkHnYvIr1oFoRwgui7P+YstDjI4MYEfLiJyH2NE5pKWVtlteSefP/krTW+t9' \
                'pxj83teo9fVge47pM5wnIcgtWk7nG99BfsmKpvcNbP4Kgz/agjgLSPzfZWhH' \
                '8bOEhOjOao+j4f0tKNY7wJuAA2+/A4ENAtd5KXoIOL7uc1sQmC/wKvEwJ5dz' \
                '1E8cwYWhqv9IVb+agsibAYfkiwSdS3DONSiBc9Y/4RxB52JK19w04aBXjx2k' \
                'NnAa07kEl8vjnEum3bnt+XCUrrmZoL15tB2Vh+l75FtEUR3rwKU6Qjxm89d9' \
                'bgvAcYGH/N9/gcD6A94PiH2ANtT2t6IbHm0Hqv6mZaj32EAOiEYGiYb6E3Ct' \
                'A+sczjlsfJkAs2AJGANjhOCcUJCjsOpSCqs2NB/MkSFGjx8irFUxHYtwhZIO' \
                '6nngWXJFWq+7FdNSOvtNzlE+sJPhw/sII0tklc8Mr3cCyzyGVRTT0yjGN+CL' \
                'eLEALELz/qDdJQe92sv518dNI7ooot5zVB0+MsDb9HKAtLQh7R0qHOdyJJ3D' \
                'lNooXXcrEjTfeqDW38PIsW6sE1yhBZnXiRMzZn+nc8GypbDuSnLLu0DOXqtz' \
                'NqL3iYeojZY9+HplaCGKXc5jeZB0rebNeIfeHHhHssrnBv/mduCw10UF1Ps3' \
                'Z+GCsO8Utlr1DNiUEX85ayFfJOhYooPoZ9Q5GVARgo5FtF7dXP3bekj5+GHC' \
                '4UGstVjnyC1YigtyKtjnUmwdlK65mVzHoqa31QZO0/f0E4RhmOxpP4YMWs4v' \
                'eCwPo9iCJoUWH3jHHcm2ql1oA0EdXZ4cb+TQhsaOZ/hByQthlej0CQ+sS35m' \
                'L0yAmdeBFIrYczmcuRwtV9+EmcCW1keGGO7e4zWUPkMwrwMptSZq9Vzw7Jwj' \
                'WLCU4vprkELz9srT2x6l2n9af/HOdexkS/IqLybt1xhEsa2jZn0taphpRdiI' \
                'IAjHEQ6sVcdBEC5DWObfO8NbEQMuqlPvOwG2juB8u7r+TP7tIkypDTN/oQqF' \
                'eCGYY29KiiXabrwDMU3aHpyj1neKyqmjCQgqtIbcwmW6vFW9Sdxce382onj5' \
                'deSXN2+zsGGN01sfxo4OY4xgRBAjGramR2SIx+5yQNZ+bgsI+xGO+fduAFpV' \
                'AHRbF1A7cdD/26C2omnNX5zDjgxhhwdIvZCMCUjMQAEzrxNMkEynOZ1VIuRj' \
                '50/OHsjZsMrQgV0Q1REHgkM8//kFiyFXSHmdS4adg2ILLZe+YEL1P3xgF+VD' \
                '+8BaAlEBMDLuY+bRuD+eAd3+AsW81aALPa/wLx5Fd8jGf+gGmgiA+IF2YZWo' \
                '75RKoHOa/cv4AbEODdrmIaU2FYo5VqpODG0vfNmEqjQcHmKkezfEwOP5xyGF' \
                'Fsy8BfrqHJsB5yz5FesorL2yqcACnN72fcL+UwSBwRghEFQLxJikFO/AFgvA' \
                'ERRjUMxLBs0OrfYvHkO3Ssd/6ComWGAgAkR1DQero/5pQJJRi82AxZTaMW3z' \
                '0wGdw0ENOhdTuuLGpt6/sxEjR/YTjQyls9+l/AuQW7AElwAyt2Jb7LqSwsr1' \
                'Te+p9fUwuHsHrlImkLHgnyE4AYphLAD9pAKwGug0IOtAWkAqIEdImj5lGcjS' \
                'RtfizEsQRAyuUsYO9iM+K5g4hFYvYmewvRPJFZlLJ8ABpatvJuhsrkptGDK0' \
                '58eaLfT8xn/B+ZA1aJuHtLR7/TBHxt85TMdiipe+AFNoacrzwK4dVI52Y0zg' \
                'Z79RzStnpOrw2C1RLEGxlSMgox7zSw261Ui8ffmxtX+W7EG3jvEPSjqDRERz' \
                '/kN9SBTpYDLGDFj1BYK2+UhLySvbuZlPEuRpfcGtmJYmDcvOMXJoL6PHD4O1' \
                'KbxjeMbk1CbPYfjqnCO/vIuWy65rfl8UMfDcdmq9xwmM8eDHGuCsKesWFEs8' \
                'tsfRpeQGWG8Q1vpPDyKcSkeRVYiPISfjcQvY8iB2dDi1YYKXTIhtq2lpwbTN' \
                'A2OS+HU2vWuHJb/2CvIrupraUmctpx7/NtVezfnrQLrk38knjSGYvwCXyyUv' \
                'zm404JBCkWLXFeQXNd8kdeTIPob3P4PgVACMB3/cyZ9cBSRzwJVwymMtCF0G' \
                'bSQUtDrUk/m+lUyy61f8/2x5CDsygDibzCTnHcI4SYRzBPMXQr4wNxVC59V/' \
                'x+Kmt4VDffRtf4zaiSMJv+LiCCBTzsZBoUgwr3NunFfrCDoWU7r25ubOn3MM' \
                '7X2WcvceckGA8eAb0V24OPtH86QNIqAYD/tPrDCkNf5RGg85iE/gmBSJiDqD' \
                'wwNQq6YOYNwvkPEJTOs8NQMiszugzhF0LKJlwzWYYnPrdXr7Y9ROn1R+K+Uk' \
                '9EtUf+bfkstj5i2Iv2JWTYEzhtyyVRTXN99GKRwZZGj3U9iRIYwxmfCvqfoH' \
                'xXBZ5vcBFGuARQZNA+NfHMrcuIAp7N2rLodgh/qxlbKvELuUsVgA0CRL0L4A' \
                'TDCrA+qiiOJl15Ff3tX8vnqd3q2PYEdHMLZO1N+DiXNdsVbK8iuCKbVBS+ss' \
                'J4a1VF26+uYJBbZ89CADzz6JyQUEZkzs3zxqzNG48/oQqQAsjsNA0M0GxgrA' \
                'lPYQEgRXHcWWBzWxEmcGY1Xq0s6hYF5nEqLNzpCqqi5umEQipXs35e49GBth' \
                'nMUO9UFUy2iBM/mVQgtB2/w00TUb5CBo76T1+lub3mbDkKG9z1A5fpicMRgh' \
                'E/5N2K8Q0LgyaIh0+/lOg9DqnYU62aPY9HWZkkNjVCKjoT5cWPXxdGNyJR5g' \
                'KRSQtnlJjD1TZ1ATKWsprr3KZ7jPTkkiJRcQGKA2ihsZ8JGAHZ/fwGBa2yGX' \
                'nz3nNZejeOm15BZd0pTf2kAvfU89jhHnQz/BmAmdv2yk3pr8MaGCEPr32uLz' \
                'cgEsQrYhbsrLvuLMoB0ZxNUqfjBpyAy6TDko6FgIRvxvM5xVDoprr6Sw+tIJ' \
                'BvN0kkgxRgiMQaIQO9jXoP6T6ibeIbQOKbUhLa2z5rxKoUjbxpchprmiHT1+' \
                'iMGd25LMn+b/sxHWhFTI/Dt75mE+rgWAVomyTzat3T7EFzVUrdbTTGCSHEoH' \
                '2JTak1TtjIbUOUzHQorrr8FMkPod3P0Uo0cPJjMpECFwDirDUC2nqaQMr3EU' \
                'YwpFTEtbQw5junw7hNzilRQ3XNvU+48qZfqeekIFVrzzB0m/4uTwbxAAR3pY' \
                'R2lO9ugXhGigD+qhDqYlUyNIw0JECNoXqBlw059Yzlnyy9bQculkEinbqPUc' \
                '83G0D6OMQLWCHRlMwBdsg+bCWl0d09qGFIoz7xQyQumG2zAtrU1vC4cGOL31' \
                '4STtq9dZiz8TUjBG2RjSc+9zNApUdXJ/spHiBCRhBVseQqyjIcTKuv0imPb5' \
                'qgJ9UmRa35kvUFhzOfklK5veVz52kOH9z2KcJfBFlNijdlGILQ8nzusZ/ILv' \
                'MGpHipNKkDYl09JG67W3IrmzW1pnIwb3PE31xKGM6vf2f2LnL0vxWQ2a9EzN' \
                '/mhOJFEHBvUY6yg24eT//rjsYwdPY9o7EIlXKqX+vtpbQfJFTGsbbtinICZt' \
                '1tInMh1Lab3m5gmnxNC+Zyl37yYINJRKNYCPYCrDuNERpLU9SW41tFk5hwQ5' \
                'glIb0eiw5jc8w1Ph2eFoueIGchNk/mxYo+eJbyNeYHMZnsVM6TtrmX/HtSOA' \
                'cKwGyBrQMtM0cbFtsqND6gzGHjVp0SXxtMVg2juTevtUv9AZQ27pxImU+sgg' \
                'g7t2EA31EwRjbakvplRHsZWhhgggrQ3Y5DKt85NoYFokhtbrbtPcwtmfjMrJ' \
                'owzu2oHBnVH3nwL4jhRj0NpArHZG4sOM4zeyq0f7gJktk4ki7FB/ZiA9P9ne' \
                'QQFTLGHyxWkIgUOKJUpX3TTBYEL5aDeDzz1JkMslGbQzGimcw5WHcWEtkxRy' \
                'Gb+FJCcgxZLnYIpi4By5pasprLliglK1o/dHW7DloSTdGwg+YTUl9R8fbRvT' \
                'PNIiX78hzf+XaBSAfs48FXvKD2uHB8BGXghsgz2NnSuCHNI2b+rD6cC0ddB6' \
                '3W1Nb7P1kOH9z1I9ccjPfhLnryHVIaJ+QLXseR1b0bRpmbg0b1qZTOcspWtu' \
                'IbdgaXPURkfoffJ7SL2eRCsNhZ/JU520xwMaBaDHoLtIgFBCMuv+hROZhMGU' \
                'r5hRV69iR4cyDl6jOnVYMEZnsAkSIZhUkiWXo7j+GnKLmydSwsHT9D39OGKt' \
                'j/31OiORYgAbYisjKrRxdRBSsyA+MdTahuTzUxsXHNI6j5bLrp/Q+x/YtZ1a' \
                'z1FEnNp9I+qrZCqtk7xChBMZXDsyyb9eg7YLO3RpcbaEdgRm6AjGOYHhAY9/' \
                'xv7H5GNtyRcRn2uf9J/PF2jdeMcEPf+O0ZNHGHxuK0HONM2ji/+/HRnE1cME' \
                'eEdmgasvFIkJkFI7+ILWZEJC7fm/ivwlayes/PX+6LvY0WECY7Ty58GfhgYI' \
                'Sdv8QDFu9yN/zOAPI0T3BFySufEwjd7jtEicxVbKPjMY59fH9g1ayOX9rJBJ' \
                'ioAQLFyusX+zREq1Qv9TT2jhxydSAsk4f2P/qoCrlrHVUZ3pziE2wydxhtBp' \
                'alhMatUmZFloufT6CTXW6InDDO97BuphEvenTuuUIagRn1estATF2gEHDLAf' \
                'TQ/OA5Yf/M074xv3M8Mza+PUMPUQOzKYzP6GokumT0AKLRoXezvbbECdCKXr' \
                'b2/e9QOEQ/30bn04caRi2980keJQfqMIEk4a8wLOOiRfmLDpNPmT1hIsWUmx' \
                '68oJU7+nn3qM8PTJJOZP4raphshKFY8lHttlKNYW2BcLQAV1AleRVgBPoBtD' \
                'zzzxbSNvV+ukpR/SSMDvJyCFIlJsmVQLtiZSXjxBIsUytP85KscOpLbf98+d' \
                'zY+OE1ludBh8NJBNU7qs5gLtbhKZUGBxjuLqKyisuqzpnVFllIFnn6Q+MpiG' \
                'q0nxZ8rwO9TJj32AAMW4hGK+1yDSi8ghb1wuIa4di1hEnkEkSgzPNK7YbhFW' \
                'cdXRhv6AJO8ev2gCpFBS7zpGYpzLiVDYcG2qSs9igF1Yo+dxn0gRSZwpySyk' \
                'OCvvtu6F1mZ8OJfMxFhSpNiqQhg/w3h/C5DWdgrrriaYvyAjQGeKzdC+Zxg9' \
                'sk/DPl+sSqKVqY9/5DGMnZhORFb49w4h0me8JOz0N6wgbR+ywDZm6gj6QXH1' \
                'GrYykiwhb8i129S+SrGUmoEmi0lL196KFFvTpejJziRpR/LoqaMM7tw2JpGi' \
                '6n9CltEex6SrqaEu4JtcHeoMFktN/QDnLLklKymuvybDb/wzXtSpnxzYuY3q' \
                'yaMEJkhbvphe3t9jt5X05NaVHmM85qNxJvAp/2KXv2IBeGI2BCCePa46CvVa' \
                'HBH5wUkrhc5aJJfXYsvZrJ1zBItXUlh1qbaj2yhzebBwWBtxetv3fSJF0jo6' \
                'maisGb+Aq1VxtdHke+MslSPuddSClrS0JWZgXDIBhRXrKCxbgwtruCjlOQ2J' \
                'hdFTRxna+zQurCTmyiQma1r2P0R3CosFYA3p9r5PAeVYALZ5SJYDaw++884Y' \
                'ot2o/ZixHyAiUKviqhUaGi18ZJCmiAVTLCFBMO7SbOcsLZffQDBvAbYeQnYw' \
                'k5+WqFKm98nvQhSOSaNOIY5yVgtaGT7BZn73zmsur2Xo8bSAc5h5nRQ3XKem' \
                'zUY4W1ch8JtquCjC4Rja9wwjB3ZqrWKsszo9FXACXRDqPKbr0AMlnMe8rCdQ' \
                'CQd84ieHLiacr6gxgvAos7AmQgR98Noo2AjBD6Jvw06LRRYKRfBt2C72EeJE' \
                'SkuJ4tqrNGdQr2OjOi6q4+r1Bk0wsHMb1VOH0RZqMvZ/Ul00aSKrWsZZbZWQ' \
                'bEQQ8+s0kSXFEm4svwJOdIeS4vpr09lf9zxHof60lvpwP0N7nqI+0EMuMN7+' \
                'MyWex1wW4TEk2eJ3PsLlHuMTCAcAa7o+sxnUU9zmb7weWOX1TQ3dLmZWtvoS' \
                'wNVGddcrSPcRsI05ATEGk285Q+qds+RXXUZuyUrN1PnZThThoghbr/ufIae3' \
                'PYwtDyeJn6SHzkxtzx8X+QimoSiUaXn3gEu+0KC1kmfOFyl2XYUptXnQ66n9' \
                'jyLd5ctZyse6Gdq1vaHka2IHcnqz3wLfAeJQZhXpDnDbgJ6uz2xO1oz1opsI' \
                'gZ4QEvsBddQPOD0rAiACYRVq1aQPn8zuItmNJaRYIrtDhgMQQ3HNFQTzF+ls' \
                'snV/NZqAyskjDO99Buq1Md7/1O2oOAuVkYy5yvJKym+QQ/LFhhxG3PXUcvnG' \
                'xPGjYR8lFVob1igf3kv50B4CE6Qmi2k7f3jMniCt53ShB1Pjse6FdNHgCLp7' \
                'RBntIL0eKHb9yWZQO7J5NgQAPyiuOgpercaNo9ll2bq5VD5twPSCEnQuJn/J' \
                'WsQY3dMvnvl+JtmoDs7R/+MnqPedTEKoIFMAn8p4Js5gWNMmV2hYQi7Z5W8m' \
                'QPJFnJg0LDWGYPEK8svWeDMVYus1bN2rfu+/1Pp7GHzuSaQeJnWKYGbOH8B3' \
                'gRMew6LHdCGpzzeSCIC/aS8+GhBhkwjLu991J+iawW8yGwkhVA27cNS3i9E4' \
                'iJm+AZzDFEo+lwBYS355F7mlq3QArQc9Hkw/u+rlIYZ2bceWB8kZQy4xAZML' \
                '/8YVAhvhqnF7hE2rmmPKxZLLY3JpXUKCHC2X676MCn6IC0MV3nrdC3Gdas8x' \
                'hnzTZ9KoYkid1qmT85gNdr/rTkRYLpLs8vo0sN9jni78EGEfev7vLf5ai9YJ' \
                'QhF+hEYEs3IQtIvquLCiIZ/n1/kIIHW2wBSKuNEAF4WYYon88rWYUjuurgeW' \
                'OWNxfnUsxkCQY/jATs38QTKTAuM7aKa755+zEFa0XcxvHCXOjmkLdUguh+Ty' \
                'yp+AaZ9Pcc0VKqzY2OUFZzQXIoKthgzv/TH1gR7yQUDO8xuHf9MRWo/VD51L' \
                'Qvi1wC1elp5AzyDWMc58aBg9BbwP3Xr8TqDVS8pB4BuzAT7gVxOX1QyMWZOX' \
                'dbJEtGUMHMGCJeQvWZd40LYeYhtmk17De56i1ns88aSNTBz3T4JhqPsIJvZd' \
                'cA1rCjX/AJIrJAtf86uvQFpa1TlNPP96ag6iOvWRQQaefjxtUjWkq5Smz/Q3' \
                'gIMeu1aPZYfH9gdkFgAlArDmgc0gPImww4/YGxEWd//WnQCDCN/J9A7M+HL1' \
                'GtRrqLPqs4CZ0NA5ixO0NmACgkWXkFt8id83N/ThX+j/XcPZiGrPUcqHdiP1' \
                'GiZeQRPEad9phVJJSIiLIKwlvDamcjMZi3xBNUEQ0LL+Bfos9RAbhUnIamNh' \
                'CGtUjndTOR7XKkiFwEyb316E7wCD3b91JwiLEd7g39uB8MOspIxtC9+F7i9v' \
                '0S1EbkPNRJw42DyTidQwqcDPKJv8HlNDI6YxBO2dFFas030IwxrWC4EOZuxU' \
                'hZQP76Fy7ECjLWVGiZQGcvUaRCEu3j4kw68uIHG6IVWQI7dkFUHHQh/3e55D' \
                '7wfEQlwdZfDZH2q0kqz2jesn0+Z3M2liL+cxvNJj+hiwa80DKYxjBaDu/8A+' \
                'SM6h9S2yHEFVyzAzpGyqlWz+nkxsHWcI0W1acsvX4sKqAu6FINktO4qIRkco' \
                'H9pDNHjah1LMNI16Js9RiAtrjWXhhF8S7SW5AoU1V0KQw4ZV7/zFQlBLHMJw' \
                'qJ/hPTt8mTrT8Tt9Zoc9Rkf8A8envhY9pg8xps2vQQDWfHozwPfRAkKE7hF4' \
                'E84JKkHfRc8QmB3yzlVDWjXuEIpj7CBHbslqTLFEVKuq3Q/DZCBj21o9dZTR' \
                '7l0ZFZrpoJ2F2Z/wW68hcQ4/myDN5ARM23zyS1drjsv7KnG8rxogxIY1hvc+' \
                'jR0ZJCC72GPKTZ9ZehjYAliP2U3oCTCRx/T7HuPxBcDTEP6sAFSC3ooxJf/B' \
                'vei5wQMzHcu4QGRrlUz4F7+n0YCGVgXyl6zVWD8ePG9PbVRP9tKvnjxM7eTh' \
                'zIofaWiAnw0SxKdz6wn/jaVd7UHIL1mF+GhFBTRMeE98gOooI3t2IDZSf8VM' \
                'u+kzpgH0/OA9az69GYwpAW/1GPagW8QPjf3QGQKw5tObQeRbiOxAxCCyCbi9' \
                '+55N8XtfReT7iLiZ9AkkVxQ1rsZJnEDNqJn2DoL5C9N8v/egNTmjYNSH+il3' \
                '70KiEBOYZEA1h2CmU0c/++UiiMKGjGB6aV0gt2g5ki/62V9TDZBoLPVXqqeO' \
                'EPYc0VL1mPTvNPh1iDyKyFfXfHoz3fdsArgNkU1+AHYg8s2xs39cAfB0Gvhr' \
                'NAm0CLibdBHpYf/eKWZISQ4grCb2NOsPiDFelbqkimbrYepFe6EI+3upHNqd' \
                '7Jyh/fMN/RizSKImyNbTIlZGEwTtnX5rWpfUJWInNQ4HieqM7H0aV60kVUot' \
                'VU+b4VPAXwGH/O+tHrNFHsO/5izp/HEFYM2nHgJV9Y+jbUS3A6/uvndT/N5X' \
                'gW8zG80izkG9njh8DWo1VyBYsNTPfE35Nswk70lXjh3AjgykS72Ikyizjr5S' \
                'FHo/ILPYBc0F5BYsRYqt2LDqs5Vhw8x3UY36UD/V4we1VJ1R/dPs+Qs9Fl9d' \
                '86mH6L53E8DPAi/12D0B/LPHbXICAIDQh/CnCEPonrNvBlb4LxgC7kfonpUy' \
                'sVNbrol2P5jiyC1c6jNr/swcH/tnQ7+oPMRo986k1Svpnzcz46tpTsDH9oJF' \
                'Yn6dQ4otuh2uiDqrNi5XK794H6BydD/RSH8S75splqrHXN3A/cCQx2YFwps9' \
                'ZkMew+zKoEkLgEPDhq/5r7od+Hkg8F+8Ffgs6ZGy0yfndFbFGUF00WVu4TJt' \
                'KPWAk8mi4f2AcOAU4eljvodOw6mkF3HGjDVhOQr9iqe4ShgRtHVgSvOwtaqP' \
                '/VXdu7DmwQ9xYZXqsX1QHdFilWSylVNneAT4LMJW/7CBx+ilHrOvA9+hWbPS' \
                '2Z8QUM/yT9DNBTuAXyI9dSoEPk96POm0SRNtvjZudd2AKbVjCiVV+WENW6um' \
                'CaC6VudstUzl0G4kqp+53m/WIW/kFxvX8uOspWBa25FcXmP/sIoLq9q0Etb0' \
                '93qNsPc49f5TiHNJ/J+CPyWu4wn6eVJTfKPHqMNj9gAw0AydswrAmvsfir/k' \
                'KeBzaALhZjSx0OF57QU+ghYfZkbOQj306wQirfk7i61ViKoVFYBaJSnN2rCK' \
                'LQ9TO3FQDz2QTBg11aGcLtXrSVNKUGjBtM7D1irYauYKqw1RS/XkYY39TVrz' \
                'P8tGzxPRbuCj4NPzCvqbPEZ1j9lTgPNYjktNdwjxHxxGTxB9xPP4C+gRcnmv' \
                'WnYAH2eGTSPinLZeRXUN44ot2LBKVBnBVsrYaln3I67poNpaherJQ36vHxoW' \
                'fsyB6z8uORsLQB3JFyDIYysj2OoItlr212jCbzQ8oGcr1KuIGN+hFPsrU+L5' \
                'tB/z7R6DPMJrgF/wGD3iMRtuBj5MIADghUAPGngAPWxgKXAPcLW/pYrwDwh/' \
                'gVCdkZPlLC4KtcvW+X686ii2Nup/VnC1CoQ1XK1CeOIg4mzqRBmm60hNyxkU' \
                'HC6qqc9RLEEUetArKqi1ivIeVqFeI+w/iR3qTRzVbL1/CjxX/Vj/A+kuLtcA' \
                '9yAs9Rg9gLB/IvBh8htBRmio8UXgXlTNvAd4D0Ivuvz4AbTr9OemIspZkrio' \
                'Uhmhdnw/TkRb8MmuvDG6aMRa7NBp3TotsfveAZwuA9Mha3WH1MFeovKw5zdu' \
                'oszwLaKrjaqjSb5Csrt9TZ6+hvpl/f5BF3ssbkIF4oseq0nt7TDpb+5+9ybQ' \
                '4+MfQA8oLgN/CHwcoeLR2YiGJHdM9u+OJU2pWz1xzDmiZC2GS1rwtJM4DvtM' \
                'UvhJc+nnlhz+hDTrD8px6EleCb96l6p8kyZ+TOwATprnLegE3Oqz5UXgvf5q' \
                'Bf4FeBewd819E89+mMJOoPc9doB337quD+EkwgsRViBc5ePQZ1GhPwnsR7iB' \
                'JmcNTXgB8UY4cVpUjFG7afwgxqdlxK3TXv2brBd4zi79TpfwKj4drZfES7wC' \
                'k1T9ktlvZDLq3yFsB96HJnYc2t798wgfQliA8AzCh4EfThZ8mIQPMA5tBj6D' \
                'NosuAz4E3OGF2GJ42L+2i2mEh9kZEVfy4tp+zkDOCHkj5ISk3y/B/Bw5f2fj' \
                'OdZAMciBQM6g/GZ3JZua6nd+LD/kx9b64Xkp8HsegxMek81T5X1KewF7LWCB' \
                'pwWWClwvsFy04eAHIn0noGSBfQJHBW4UWDgdBRALQVzSzaZ444xf/LreNz1l' \
                'M1tXlmdJeE4XdSZZyjH8TiJhuVfgA6jtr4v0AaXrBO734z8q8AXgE0B1KrN/' \
                'ygIAcN+jB3jbrevqLfAjtNf8WnTR4Rpo/ZEgvYAVldrDaDvy4ql8RzKome6Y' \
                'rLcs2UGcxaaPmdC4PJOmdxOBmBq/O4HfBf4JqOunSlegvtedqHb4O+CDgzB4' \
                '2SenBn7M97To0HvuAhWAPwVehXqdXwHeh7AHAEeAOowfI12VcpEmR9uBDwL/' \
                'QryHs+NS4A+A/4RO3m8Cvw4cXP3J70zrS2a6VWw38H7g3z1Dr0f4KBDv2Byh' \
                'LUrvZhb7CX8CaDM6Zt8gDec2IHwEeD061v+OOoXd0/mCmKZsAmK679H9vOfW' \
                'dZDjBI7n0IWH61F/YDWapeoFHHdzgK1sR1iEnkYanDPj/Py6agj/iPABlvMY' \
                'w8kq+SuA3wfe6L3/7yK8nxxbsbjpzv4ZCUAiBC9eB3re4HPAejRXcKX/+WOE' \
                'k2wFtDjxBBouXoVuU3KRUupDzenHgJ2M4DNbXI/a/NeieG1G/YIf4GYGPsxQ' \
                'AMALwUvWOfRAwmfQbOB61AzcAOxFOIwCP4AuPulGhWQh59d3uxDIooWdDwN/' \
                'hk4UEHJoQu0+1OETNMP3fvymD6s/MTPwYRYEABqE4BjCdoQ1CBsQuhBeDPQg' \
                '7PbOTAUVlEd9MmklUzie7j/YNYLwb2h271+BEf96Efh5hD9CD/aOEL6F8F60' \
                '539WwJ81AWgQAsNJdHHJMtQMXIIuTiiiJqGMEGE4CvwbWkfYgG5KMSfnF1yA' \
                'VEd3Z/tj4CMY9gCR14WLUQfwg+iavjLw98BvI+zE4WYLfJhFAYBECEBV/RbU' \
                'zm9AheEWdH+aA2gTo/UP9wN0LUK8UWX7rA/3hUUn0E6d36GxVTtATeaHgbej' \
                'y/RPog0fH0RN7KyCD3Nofw/99l2gmxK/FXgn6viBAv4pPwj9mY+0A69Dmxpe' \
                'TONRZ/8RqB9tsv0SmtjJrrDqBF6Nltlv8q89ixbePg9UVv/x7AIf06TPBZwm' \
                'VdC+wd1oyfLl/gHvQzXCFyFZrDgMfBnVHK8HXoP2IT7fNcIw2qDxzyjwh8e8' \
                '/yJ0Cd4vohow7vL9JLrFy8y27J+A5twD95oANCq4G3gzuhsZ6Iz4MvB3CMeA' \
                'bPloA+r9vgq4C40Ynk90GgXwm+iSOs2OxiPuuARt4PxldDKARgB/CTwY3z9X' \
                'Mz+mcxKCZYRgPvBTCO8k7VvvR9e0fQkteJTH1BBXo7bx5Qg/A1x2rvieBunW' \
                'eo5/QWfxNtLFGjHXrah2exOq4TrRWf49HH+COsaDMPfgpyydIzr023dp91qd' \
                '9X4A3o5GCYI6OY+g0v8wMJrsHe8AQyfqRL4I1QovIz3d9HxFD5b0RI4taEOG' \
                '5jls0rETJ3RKKOB3o1HRCv9kx/wzf5Ec+whx5wL4mM75TDr0O4k2aEcrie9C' \
                'Z8R8P6Cn0HbnB/1gjiBSz7TW5P29cWSxCV3FHB92nWfufJs6aqNDSMLdh1BT' \
                'dhKNfrRFW1e85IA2VGjfhpqyJajADqIa7wF0355hgNV/dO7Ah/OkSjNCIGg7' \
                '88uB/+oHagEqCIOoRvgyIo+gK1x1EUoqDPFBV63o/kU3o+biKnSgW9BDE7OC' \
                'ETR5bofO6CzQNdSZPYV65tvQlPYuNIytEq+5Txs82oDFOHcb2qd/G2meow8V' \
                '7M+iZmLAf+85Bz8G4LxQRggADCIdaBj4K6hmiB3FGjrYf4/OtgOos9R4rqGu' \
                'hzfJJbIMTUuvQvsVlqHC1Ymq4xKppqijJ2qPoj5JHxqvH0G99v04d4K03zNe' \
                'E5aloud5LaqVfg4VyvjUzuPoTP8r4Ks4N0BmA87zAT5cQM7Uof/x8vifi4Cf' \
                'RmveG9FoIOZzAFW3D6H18oMoQIPnie35qIB1oUWbTahZ6vDvO3RPha1o0udb' \
                '+A0aV//ht88Ty410wQhATBlBaEdVZzyoGyFzqJXO0qdRQdiFaoaD6KztnSP2' \
                'FpF0P7EOneHXoxprQea+ART0WFgfIbbxFwjwMV1wAhBTRhBAF6HcCLwQdfiu' \
                'Jz36LKbDpAJw3P8en3rSg6r2IX9VUVsfq3FBfYMiepzKPNRULEZ9iWXoTF+O' \
                'CkCX/z1LFVQYH0Xb5Z5Ei17AhQd8TBesAMQ0RhDaUJOwAXX2bkY1w7JxPlpB' \
                'TUMMetm/VkFtviXdMCmH+g45VLBaUMcyFob5nClwoAK2FXUKt6Hqfi+ZFdMX' \
                'KvAxXfACENMYQQAFaAk6S7tQgXgB2mewCgVsMudDNKN4YY9FBecw2qi5AwX8' \
                'IKpdTtF4POsFD3xMzxsByNI4wiCodmhFvfsFvj1tHSocq1FBWYCmlFvRsDDu' \
                'ShpFQ74ymsLtQ4E9hIK8H8c+//qov2+EMeseni+gZ+n/A+i70Hi4p0g8AAAA' \
                'JXRFWHRkYXRlOmNyZWF0ZQAyMDEzLTEwLTA5VDA5OjI4OjM1KzA4OjAwLjA0' \
                'YQAAACV0RVh0ZGF0ZTptb2RpZnkAMjAxMy0xMC0wOVQwOToyODozNSswODow' \
                'MF9tjN0AAAAZdEVYdFNvZnR3YXJlAEFkb2JlIEltYWdlUmVhZHlxyWU8AAAA' \
                'AElFTkSuQmCC'

    source_dir = os.path.abspath(source_dir)
    manifest_path = os.path.join(source_dir, 'manifest.json')
    manifest_data = json.load(open(manifest_path))
    has_custom_icon = True

    try:
      icon_path = manifest_data['icons']['128']
      open(os.path.join(source_dir, icon_path))
    except (KeyError, IOError):
      has_custom_icon = False

    if has_custom_icon: # Nothing to do, user has his own icon.
      return

    # Generate a standard icon into the desired location.
    print('No 128x128 found in the manifest file, using the default one.')
    icon_file = open(os.path.join(source_dir, 'icon-128.png'), 'w')
    icon_file.write(image_str.decode('base64'))

    # Set the default icon in manifest.json.
    icon_data = manifest_data.get('icons', {})
    icon_data['128'] = 'icon-128.png'
    manifest_data['icons'] = icon_data

    json.dump(manifest_data, open(manifest_path, 'w'), indent=2)

  @classmethod
  def __Compress(cls, src, dst):
    try:
      print('Adding resources from %s into package.' % src)
      zfile = zipfile.ZipFile(dst, 'w')
      abs_src = os.path.abspath(src)
      for dirname, _, files in os.walk(src):
        for filename in files:
          absname = os.path.abspath(os.path.join(dirname, filename))
          relativename = absname[len(abs_src) + 1:]
          zfile.write(absname, relativename)
      zfile.close()
      print('Generated package successfully.')
    except IOError:
      if os.path.exists(dst):
        os.remove(dst)
      traceback.print_exc()

def main():
  parser = argparse.ArgumentParser(
      description='XPKGenerator arguments parser')
  parser.add_argument('input',
      help='Directory path to Crosswalk package resources')
  parser.add_argument(
      'key',
      help='Path to private key file, a new private ' \
           'key file will be generated if it is invalid.')
  parser.add_argument(
      '-o', '--output',
      help='Path to generated XPK file',
      default='default')
  args = parser.parse_args()

  output_file = args.output
  if output_file == 'default':
    head, tail = os.path.split(args.input)
    while len(tail) == 0:
      head, tail = os.path.split(head)
    output_file = tail + '.xpk'
  generator = XPKGenerator(args.input, args.key, output_file)
  generator.Generate()

if __name__ == '__main__':
  main()
