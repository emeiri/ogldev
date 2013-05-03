struct VSInput
{
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;  
};

interface VSOutput
{
    vec3 WorldPos;
    vec2 TexCoord;
    vec3 Normal;  
};


uniform mat4 gWVP;
uniform mat4 gWorld;
                    
                    
shader VSmain(in VSInput VSin:0, out VSOutput VSout)         
{                   
    gl_Position = gWVP * vec4(VSin.Position, 1.0);
    VSout.TexCoord   = VSin.TexCoord;                  
    VSout.Normal     = (gWorld * vec4(VSin.Normal, 0.0)).xyz;   
    VSout.WorldPos   = (gWorld * vec4(VSin.Position, 1.0)).xyz; 
};


struct FSOutput
{                   
    vec3 WorldPos;    
    vec3 Diffuse;     
    vec3 Normal;      
    vec3 TexCoord;    
};

											
uniform sampler2D gColorMap;                
											
shader FSmain(in VSOutput FSin, out FSOutput FSout)									
{											
	FSout.WorldPos = FSin.WorldPos;					
	FSout.Diffuse  = texture(gColorMap, FSin.TexCoord).xyz;	
	FSout.Normal   = normalize(FSin.Normal);					
	FSout.TexCoord = vec3(FSin.TexCoord, 0.0);				
};

program GeometryPass
{
    vs(410)=VSmain();
    fs(410)=FSmain();
};
