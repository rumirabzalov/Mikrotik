# BEGIN SETUP
:local myserver ([/system identity get name])
:local scheduleName "LogMikrotik"
#Write Bot TOKEN
local bot "123456789:TOKEN"
#Write Group or chat
local ChatID "-123456789"
:local startBuf [:toarray [/log find message~"logged in" || message~"login failure" || message~" failure" || message~"down" || message~"link up" || message~"fcs" || message~"excessive" ]]
:local removeThese {"telnet";"whatever string you want"}
# END SETUP

# warn if schedule does not exist
:if ([:len [/system scheduler find name="$scheduleName"]] = 0) do={
  /log warning "[LOGMON] ERROR: Schedule does not exist. Create schedule and edit script to match name"
}

# get last time
:local lastTime [/system scheduler get [find name="$scheduleName"] comment]
# for checking time of each log entry
:local currentTime
# log message
:local message
 
# final output
:local output

:local keepOutput false
# if lastTime is empty, set keepOutput to true
:if ([:len $lastTime] = 0) do={
  :set keepOutput true
}

:local counter 0
# loop through all log entries that have been found
:foreach i in=$startBuf do={
 
# loop through all removeThese array items
  :local keepLog true
  :foreach j in=$removeThese do={
#   if this log entry contains any of them, it will be ignored
    :if ([/log get $i message] ~ "$j") do={
      :set keepLog false
    }
  }
  :if ($keepLog = true) do={
   
   :set message [/log get $i message]

#   LOG DATE
#   depending on log date/time, the format may be different. 3 known formats
#   format of jan/01/2002 00:00:00 which shows up at unknown date/time. Using as default
    :set currentTime [ /log get $i time ]
#   format of 00:00:00 which shows up on current day's logs
   :if ([:len $currentTime] = 8 ) do={
     :set currentTime ([:pick [/system clock get date] 0 11]." ".$currentTime)
    } else={
#     format of jan/01 00:00:00 which shows up on previous day's logs
     :if ([:len $currentTime] = 15 ) do={
        :set currentTime ([:pick $currentTime 0 6]."/".[:pick [/system clock get date] 7 11]." ".[:pick $currentTime 7 15])
      }
   }
    
#   if keepOutput is true, add this log entry to output
   :if ($keepOutput = true) do={
     :set output ($output.$currentTime." ".$message."\r\n")
   }

    :if ($currentTime = $lastTime) do={
     :set keepOutput true
     :set output ""
   }
  }
  :if ($counter = ([:len $startBuf]-1)) do={
   :if ($keepOutput = false) do={    
     :if ([:len $message] > 0) do={
        :set output ($output.$currentTime." ".$message."\r\n")
      }
    }
  }
  :set counter ($counter + 1)
}

if ([:len $output] > 0) do={
  /system scheduler set [find name="$scheduleName"] comment=$currentTime
  /tool fetch url="https://api.telegram.org/bot$bot/sendmessage?chat_id=$ChatID&text=$myserver : $output" keep-result=no;
}