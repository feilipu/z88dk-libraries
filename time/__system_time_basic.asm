;
; (c)2017 Phillip Stevens All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are
; met:
;
; Redistributions of source code must retain the above copyright notice, this
; list of conditions and the following disclaimer. Redistributions in binary
; form must reproduce the above copyright notice, this list of conditions
; and the following disclaimer in the documentation and/or other materials
; provided with the distribution. Neither the name of the copyright holders
; nor the names of contributors may be used to endorse or promote products
; derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;

;
;	The system time stamp, when run under YAZ180 NASCOM Basic 56k.
;
    PUBLIC __system_time_fraction_basic, __system_time_basic

DEFC    __system_time_fraction_basic    =   $2053   ; uint8_t (1/256) fractional time
DEFC    __system_time_basic             =   $2054   ; time_t

