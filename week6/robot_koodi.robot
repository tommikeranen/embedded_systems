#Tavoitteen on 1 piste

*** Settings ***
Library   String
Library   SerialLibrary

*** Variables ***
${com}   	COM4
${baud} 	115200
${board}	nRF
${seq1}      RYGX
${return1}    -1X
${seq2}      000120X
${return2}    80X
${seq3}      006705X
${return3}    -3X

*** Test Cases ***
Connect Serial
	Log To Console  Connecting to ${board}
	Add Port  ${com}  baudrate=${baud}  encoding=ascii
	Port Should Be Open  ${com}
	Reset Input Buffer
	Reset Output Buffer

LEN_ERROR_test
	# lähetetään RYG ja lopetusmerkki X
	Write Data   ${seq1}   encoding=ascii 
	Log To Console   Send sequence ${seq1}

	# vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	${read} =   Read Until   terminator=58   encoding=ascii 

	# konsolille näkyviin vastaanotettu merkkijono
	Log To Console   Received ${read}
	
	# vertaillaan merkkijonoa
	Should Be Equal As Strings   ${read}    ${return1}
	Log To Console   Tested ${read} is same as ${return1}

correct_test
	# lähetetään RYG ja lopetusmerkki X
	Write Data   ${seq2}   encoding=ascii 
	Log To Console   Send sequence ${seq2}

	# vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	${read} =   Read Until   terminator=58   encoding=ascii 

	# konsolille näkyviin vastaanotettu merkkijono
	Log To Console   Received ${read}
	
	# vertaillaan merkkijonoa
	Should Be Equal As Strings   ${read}    ${return2}
	Log To Console   Tested ${read} is same as ${return2}

VALUE_ERROR_test
	# lähetetään RYG ja lopetusmerkki X
	Write Data   ${seq3}   encoding=ascii 
	Log To Console   Send sequence ${seq3}

	# vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	${read} =   Read Until   terminator=58   encoding=ascii 

	# konsolille näkyviin vastaanotettu merkkijono
	Log To Console   Received ${read}
	
	# vertaillaan merkkijonoa
	Should Be Equal As Strings   ${read}    ${return3}
	Log To Console   Tested ${read} is same as ${return3}

	
Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}