interface VSOutput
{
    vec3 WorldSpacePos;
    vec2 TexCoord;
    vec3 Normal;  
};


uniform mat4 gWVP;
uniform mat4 gWorld;
                                        
shader VSmain(in vec3 Pos, in vec2 TexCoord, in vec3 Normal, out VSOutput VSout)         
{       
    gl_Position         = gWVP * vec4(Pos, 1.0);
    VSout.TexCoord      = TexCoord;                  
    VSout.Normal        = (gWorld * vec4(Normal, 0.0)).xyz;   
    VSout.WorldSpacePos = (gWorld * vec4(Pos, 1.0)).xyz;   
};


struct FSOutput
{                   
    vec3 WorldSpacePos;    
    vec3 Diffuse;     
    vec3 Normal;      
    vec3 TexCoord;    
};
											
uniform sampler2D gColorMap;                
											
shader FSmain(in VSOutput FSin, out FSOutput FSout)									
{											
	FSout.WorldSpacePos = FSin.WorldSpacePos;					
	FSout.Diffuse      = texture(gColorMap, FSin.TexCoord).xyz;	
	FSout.Normal       = normalize(FSin.Normal);					
	FSout.TexCoord     = vec3(FSin.TexCoord, 0.0);				
};

program GeometryPass
{
    vs(410)=VSmain();
    fs(410)=FSmain();
};
